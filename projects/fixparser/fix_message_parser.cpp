#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <hffix.hpp>

#include <string>
#include <string_view>

#include "fix_message_parser.h"
#include "errors.h"

Expected<FixMessage> FixMessageParser::Parse(std::string_view fix_message) {
  hffix::message_reader reader(fix_message.data(), fix_message.size());

  if (!reader.is_complete()) {
    return Unexpected<>{MakeErrorCode(Error::kReaderIsNotComplete)};
  }

  if (!reader.is_valid()) {
    return Unexpected<>{MakeErrorCode(Error::kReaderIsNotValid)};
  }

  if (reader.message_type()->value() == "A") {
    return OnLogon(reader);
  } else if (reader.message_type()->value() == "5") {
    return OnLogout(reader);
  } else if (reader.message_type()->value() == "0") {
    return OnHeartbeat(reader);
  } else if (reader.message_type()->value() == "1") {
    return OnTestRequest(reader);
  } else if (reader.message_type()->value() == "V") {
    return OnMarketDataRequest(reader);
  } else if (reader.message_type()->value() == "Y") {
    return OnMarketDataRequestReject(reader);
  } else if (reader.message_type()->value() == "W") {
    return OnMarketDataSnapshotFullRefresh(reader);
  }

  return Unexpected<>{MakeErrorCode(Error::kReaderUnhandledMessageType)};
}

Expected<FixMessage> FixMessageParser::Parse(const std::string& fix_message) {
  return Parse(std::string_view{fix_message});
}

Expected<FixMessageHeader> FixMessageParser::ParseFixHeader(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  const auto sender = Sender(reader, hint);

  if (!sender) {
    return Unexpected<>{sender.error()};
  }

  const auto receiver = Receiver(reader, hint);

  if (!receiver) {
    return Unexpected<>{receiver.error()};
  }

  const auto msg_seq_num = MsgSeqNum(reader, hint);

  if (!msg_seq_num) {
    return Unexpected<>{msg_seq_num.error()};
  }

  const auto sending_time = SendingTime(reader, hint);

  if (!sending_time) {
    return Unexpected<>{sending_time.error()};
  }

  return FixMessageHeader{
    *sender,
    *receiver,
    *sending_time,
    *msg_seq_num
  };
}

Expected<Logon> FixMessageParser::OnLogon(hffix::message_reader& reader) {
  auto hint = reader.begin();

  const auto header = ParseFixHeader(reader, hint);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  const auto heartbeat_interval = HeartbeatInterval(reader, hint);

  if (!heartbeat_interval) {
    return Unexpected<>{heartbeat_interval.error()};
  }

  return Logon{*header, *heartbeat_interval};
}

Expected<Logout> FixMessageParser::OnLogout(hffix::message_reader& reader) {
  auto hint = reader.begin();

  const auto header = ParseFixHeader(reader, hint);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  return Logout{*header};
}

Expected<Heartbeat> FixMessageParser::OnHeartbeat(hffix::message_reader& reader) {
  auto hint = reader.begin();

  const auto header = ParseFixHeader(reader, hint);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  return Heartbeat{*header};
}

Expected<TestRequest> FixMessageParser::OnTestRequest(hffix::message_reader& reader) {
  auto hint = reader.begin();

  const auto header = ParseFixHeader(reader, hint);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  return TestRequest{*header};
}

Expected<MarketDataRequest> FixMessageParser::OnMarketDataRequest(hffix::message_reader& reader) {
  return {};
}

Expected<MarketDataRequestReject> FixMessageParser::OnMarketDataRequestReject(hffix::message_reader& reader) {
  auto hint = reader.begin();

  const auto header = ParseFixHeader(reader, hint);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  const auto md_req_id = MdReqId(reader, hint);

  if (!md_req_id) {
    return Unexpected<>{md_req_id.error()};
  }

  return MarketDataRequestReject{
    *header,
    *md_req_id
  };
}

Expected<MarketDataSnapshotFullRefresh> FixMessageParser::OnMarketDataSnapshotFullRefresh(hffix::message_reader& reader) {
  auto hint = reader.begin();

  auto header = ParseFixHeader(reader, hint);

  if (!header) {
    return Unexpected<>{header.error()};
  }

  auto symbol = Symbol(reader, hint);

  if (!symbol) {
    return Unexpected<>{symbol.error()};
  }

  auto md_req_id = MdReqId(reader, hint);

  if (!md_req_id) {
    return Unexpected<>{md_req_id.error()};
  }

  auto security_type = SecurityType(reader, hint);

  if (!security_type) {
    return Unexpected<>{security_type.error()};
  }

  auto security_group = SecurityGroup(reader, hint);

  if (!security_group) {
    return Unexpected<>{security_group.error()};
  }

  auto no_md_entries = NoMdEntries(reader, hint);

  if (!no_md_entries) {
    return Unexpected<>{no_md_entries.error()};
  }

  auto settlement_date = SettlementDate(reader, hint);
  auto tenor = Tenor(reader, hint);

  if (!tenor && !settlement_date) {
    return Unexpected<>{MakeErrorCode(Error::kNotFoundNorTenorNorSettlementDate)};
  }

  MarketDataSnapshotFullRefresh result{
    std::move(*header),
    std::move(*symbol),
    std::move(*security_type),
    std::move(*security_group),
    std::move(*tenor),
    std::move(*settlement_date),
    std::move(*md_req_id),
    {}
  };

  for (uint64_t j = 0; j < *no_md_entries; ++j) {
    OrderBookLevel level{};

    const auto md_entry_type = MdEntryType(reader, hint);

    if (!md_entry_type) {
      return Unexpected<>{md_entry_type.error()};
    }

    if (md_entry_type == '0') {
      level.type = OrderBookLevelType::kBid;
    } else if (md_entry_type == '1') {
      level.type = OrderBookLevelType::kOffer;
    } else {
      // log warn/error
      return Unexpected<>{MakeErrorCode(Error::kUnhandledMdEntryTypeValue)};
    }

    const auto md_entry_px = MdEntryPx(reader, hint);

    if (!md_entry_px) {
      return Unexpected<>{md_entry_px.error()};
    }

    const auto md_entry_size = MdEntrySize(reader, hint);

    if (!md_entry_size) {
      return Unexpected<>{md_entry_size.error()};
    }

    level.price = *md_entry_px;
    level.volume = *md_entry_size;
    result.levels.push_back(level);
  }

  return result;
}

Expected<std::string> FixMessageParser::Symbol(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::Symbol, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundSymbol)};
}

Expected<std::string> FixMessageParser::Sender(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::SenderCompID, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundSenderCompId)};
}

Expected<std::string> FixMessageParser::Receiver(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::TargetCompID, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundTargetCompId)};
}

Expected<uint64_t> FixMessageParser::HeartbeatInterval(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::HeartBtInt, hint)) {
    return hint->value().as_int<uint64_t>();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundHeartbeatInterval)};
}

Expected<std::string> FixMessageParser::SecurityType(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::SecurityType, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundSecurityType)};
}

Expected<std::string> FixMessageParser::SecurityGroup(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::SecurityGroup, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundSecurityGroup)};
}

Expected<uint64_t> FixMessageParser::MsgSeqNum(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::MsgSeqNum, hint)) {
    return hint->value().as_int<uint64_t>();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundMsgSeqNum)};
}

Expected<std::string> FixMessageParser::SendingTime(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::SendingTime, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundSendingTime)};
}

Expected<uint64_t> FixMessageParser::NoMdEntries(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::NoMDEntries, hint)) {
    return hint->value().as_int<uint64_t>();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundNoMdEntries)};
}

Expected<std::string> FixMessageParser::Tenor(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(6215, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundTenor)};
}

Expected<std::string> FixMessageParser::SettlementDate(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::SettlDate, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundSettlementDate)};
}

Expected<std::string> FixMessageParser::MdReqId(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::MDReqID, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundMdReqId)};
}

Expected<char> FixMessageParser::MdEntryType(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::MDEntryType, hint)) {
    return hint->value().as_char();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundMdEntryType)};
}

Expected<std::string> FixMessageParser::MdEntryPx(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::MDEntryPx, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundMdEntryPx)};
}

Expected<uint64_t> FixMessageParser::MdEntrySize(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::MDEntrySize, hint)) {
    return hint->value().as_int<uint64_t>();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundMdEntrySize)};
}

Expected<std::string> FixMessageParser::SubscriptionRequestType(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint) {
  if (reader.find_with_hint(hffix::tag::SubscriptionRequestType, hint)) {
    return hint->value().as_string();
  }

  return Unexpected<>{MakeErrorCode(Error::kNotFoundSubscriptionRequestType)};
}
