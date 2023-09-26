#include "my_fix_parser.h"
#include "fields.h"
#include "errors.h"

namespace {

using namespace my;

/*
\brief Internal ascii-to-integer conversion.

Parses ascii and returns a (possibly negative) integer.

\tparam Int_type The type of integer to be returned.
\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\return The ascii string represented as an integer of type Int_type.
*/
template<typename Int_type>
Int_type atoi(const char* begin, const char* end) {
  Int_type val{};
  bool isnegative{};

  if (begin < end && *begin == '-') {
    isnegative = true;
    ++begin;
  }

  for(; begin < end; ++begin) {
    val *= 10;
    val += (Int_type)(*begin - '0');
  }

  return isnegative ? -val : val;
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>, void>>
Expected<T> StringToInteger(const char* begin, const char* end) {
  T value{};
  bool is_negative{};

  if (begin < end && *begin == '-') {
    is_negative = true;
    ++begin;
  }

  const auto is_valid_non_negative = std::is_unsigned_v<T> && is_negative;

  if (is_valid_non_negative) {
    return Unexpected<>{MakeErrorCode(Error::kConvertingNegativeStringNumberToUnsignedNumber)};
  }

  for (; begin < end; ++begin) {
    const auto is_digit = *begin >= '0' && *begin <= '9';

    if (!is_digit) {
      return Unexpected<>{MakeErrorCode(Error::kStringValueIsNotNumberParsingError)};
    }

    value = value * 10 + (*begin - '0');
  }

  return value;
}

/*
\brief Internal ascii-to-date conversion.

Parses ascii and returns a LocalMktDate or UTCDate.

\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\param[out] year Year.
\param[out] month Month.
\param[out] day Day.
\return True if successful and the out arguments were set.
 */
inline bool atodate(
  char const* begin,
  char const* end,
  int& year,
  int& month,
  int& day
) {
  if (end - begin != 8) return false;

  year = atoi<int>(begin, begin + 4);
  month = atoi<int>(begin + 4, begin + 6);
  day = atoi<int>(begin + 6, begin + 8);

  return true;
}

/*
\brief Internal ascii-to-time conversion.

Parses ascii and returns a time.

\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\param[out] hour Hour.
\param[out] minute Minute.
\param[out] second Second.
\param[out] millisecond Millisecond.
\return True if successful and the out arguments were set.
*/
inline bool atotime(
  char const* begin,
  char const* end,
  int& hour,
  int& minute,
  int& second,
  int& millisecond
) {
  if (end - begin != 8 && end - begin != 12) {
    return false;
  }

  hour = atoi<int>(begin, begin + 2);
  minute = atoi<int>(begin + 3, begin + 5);
  second = atoi<int>(begin + 6, begin + 8);

  if (end - begin == 12) {
    millisecond = atoi<int>(begin + 9, begin + 12);
  } else {
    millisecond = 0;
  }

  return true;
}

/*
\brief Internal ascii-to-timepoint conversion.

Parses ascii and returns a std::chrono::time_point.

\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\param[out] tp time_point.
\return True if successful and the out arguments were set.
*/
template <typename TimePoint>
inline bool atotimepoint(
  char const* begin,
  char const* end,
  TimePoint& tp
) {
  // TODO: after c++20 this simplifies to
  // std::chrono::parse("%Y%m%d-%T", tp);
  int year, month, day, hour, minute, second, millisecond;

  if (!atotime(begin + 9, end, hour, minute, second, millisecond)) {
    return false;
  }

  if (!atodate(begin, begin + 8, year, month, day)) {
    return false;
  }

  // from http://howardhinnant.github.io/date_algorithms.html
  year -= month <= 2;
  const unsigned era = (year >= 0 ? year : year - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(year - era * 400);
  const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  const unsigned long days_since_epoch = era * 146097 + static_cast<unsigned>(doe) - 719468;

  tp = TimePoint(
    std::chrono::seconds(days_since_epoch * 24 * 3600) +
    std::chrono::hours(hour) +
    std::chrono::minutes(minute) +
    std::chrono::seconds(second) +
    std::chrono::milliseconds(millisecond));

  return true;
}

}

namespace my {

MyFixParser::MyFixParser() {
  tag::DictionaryInitField(field_dictionary_);
  tag::DictionaryInitMessage(message_name_dictionary_);
}

Expected<FixMessage> MyFixParser::Parse(std::string_view fix_message) {
  MessageLabels labels;

  for (size_t i = 0; i < fix_message.size();) {
    auto equal_sign_index = fix_message.find('=', i);

    if (equal_sign_index == std::string::npos) {
      return Unexpected<>{MakeErrorCode(Error::kNotFoundEqualSignInFieldParseError)};
    }

    const auto field = StringToInteger<int>(
      fix_message.data() + i,
      fix_message.data() + i + equal_sign_index);

    if (!field) {
      return Unexpected<>{field.error()};
    }

    auto field_delimiter_index = fix_message.find('\x01', equal_sign_index + 1);

    if (field_delimiter_index == std::string::npos) {
      return Unexpected<>{MakeErrorCode(Error::kNotFoundFieldDelimiterParseError)};
    }

    const char* field_value_begin = fix_message.data() + equal_sign_index + 1;
    labels[static_cast<tag::Field>(*field)] = std::string_view{field_value_begin, field_delimiter_index - equal_sign_index - 1};

    i = field_delimiter_index + 1;
  }

  const auto msg_type_it = labels.find(tag::MsgType);

  if (msg_type_it == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundMsgType)};
  }

  if (msg_type_it->second == "A") {
    return OnLogon(labels);
  } else if (msg_type_it->second == "5") {
    return OnLogout(labels);
  } else if (msg_type_it->second == "0") {
    return OnHeartbeat(labels);
  } else if (msg_type_it->second == "1") {
    return OnTestRequest(labels);
  } else if (msg_type_it->second == "3") {
    return OnReject(labels);
  } else if (msg_type_it->second == "V") {
    return OnMarketDataRequest(labels);
  } else if (msg_type_it->second == "Y") {
    return OnMarketDataRequestReject(labels);
  } else if (msg_type_it->second == "W") {
    return OnMarketDataSnapshotFullRefresh(labels);
  }

  return Unexpected<>{MakeErrorCode(Error::kUnhandledMessageType)};
}

Expected<FixMessageHeader> MyFixParser::ParseFixHeader(const MyFixParser::MessageLabels& labels) {
  const auto sender_it = labels.find(tag::SenderCompID);
  const auto target_it = labels.find(tag::TargetCompID);
  const auto msg_seq_num_it = labels.find(tag::MsgSeqNum);
  const auto sending_time_it = labels.find(tag::SendingTime);

  if (sender_it == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundSenderCompId)};
  }

  if (target_it == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundTargetCompId)};
  }

  if (msg_seq_num_it == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundMsgSeqNum)};
  }

  if (sending_time_it == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundSendingTime)};
  }

  TimePoint<Milliseconds> tp;

  atotimepoint(
    sending_time_it->second.data(),
    sending_time_it->second.data() + sending_time_it->second.size(), tp);

  const auto msg_seq_num = StringToInteger<uint64_t>(
    msg_seq_num_it->second.data(),
    msg_seq_num_it->second.data() + msg_seq_num_it->second.size());

  if (!msg_seq_num) {
    return Unexpected<>{msg_seq_num.error()};
  }

  return FixMessageHeader{
    std::string{sender_it->second},
    std::string{target_it->second},
    tp,
    *msg_seq_num
  };
}

Expected<Logon> MyFixParser::OnLogon(const MyFixParser::MessageLabels& labels) {
  auto header = ParseFixHeader(labels);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  const auto heart_bt_int_it = labels.find(tag::HeartBtInt);

  if (heart_bt_int_it == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundHeartbeatInterval)};
  }

  const auto heart_bt_int = StringToInteger<uint64_t>(
    heart_bt_int_it->second.data(),
    heart_bt_int_it->second.data() + heart_bt_int_it->second.size());

  if (!heart_bt_int) {
    return Unexpected<>{heart_bt_int.error()};
  }

  return Logon{
    std::move(*header),
    *heart_bt_int
  };
}
Expected<Logout> MyFixParser::OnLogout(const MyFixParser::MessageLabels& labels) {
  auto header = ParseFixHeader(labels);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  return Logout{
    *header
  };
}

Expected<Heartbeat> MyFixParser::OnHeartbeat(const MyFixParser::MessageLabels& labels) {
  auto header = ParseFixHeader(labels);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  return Heartbeat{
    *header
  };
}

Expected<TestRequest> MyFixParser::OnTestRequest(const MyFixParser::MessageLabels& labels) {
  auto header = ParseFixHeader(labels);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  return TestRequest{
    *header
  };
}

Expected<Reject> MyFixParser::OnReject(const MyFixParser::MessageLabels& labels) {
  auto header = ParseFixHeader(labels);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  return Reject{
    *header
  };
}

Expected<MarketDataRequest> MyFixParser::OnMarketDataRequest(const MyFixParser::MessageLabels& labels) {
  auto header = ParseFixHeader(labels);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  const auto md_req_id = labels.find(tag::MDReqID);
  const auto subscription_request_type = labels.find(tag::SubscriptionRequestType);
  const auto no_related_sym = labels.find(tag::NoRelatedSym);

  if (md_req_id == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundMdReqId)};
  }

  if (subscription_request_type == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundSubscriptionRequestType)};
  }

  if (no_related_sym == labels.end()) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundNoRelatedSym)};
  }

  std::vector<MarketDataRequest::Instrument> instruments;

  /*for (uint64_t i = 0; i < *no_related_sym; ++i) {

  }*/

  return my::Expected<MarketDataRequest>();
}

Expected<MarketDataRequestReject> MyFixParser::OnMarketDataRequestReject(const MyFixParser::MessageLabels&) {
  return my::Expected<MarketDataRequestReject>();
}

Expected<MarketDataSnapshotFullRefresh> MyFixParser::OnMarketDataSnapshotFullRefresh(const MyFixParser::MessageLabels&) {
  return my::Expected<MarketDataSnapshotFullRefresh>();
}

}