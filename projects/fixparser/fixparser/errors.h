#pragma once

#include <boost/system/error_code.hpp>
#include <system_error>

enum class Error {
  kReaderIsNotComplete,
  kReaderIsNotValid,
  kUnhandledMessageType,
  kNotFoundMsgType,
  kNotFoundSymbol,
  kNotFoundAccount,
  kNotFoundSenderCompId,
  kNotFoundTargetCompId,
  kNotFoundSecurityType,
  kNotFoundSecurityGroup,
  kNotFoundMsgSeqNum,
  kNotFoundSendingTime,
  kNotFoundLastUpdateTime,
  kNotFoundNoMdEntries,
  kNotFoundTenor,
  kNotFoundSettlementDate,
  kNotFoundMdReqId,
  kNotFoundSubscriptionRequestType,
  kNotFoundMdEntryType,
  kNotFoundHeartbeatInterval,
  kNotFoundNorTenorNorSettlementDate,
  kNotFoundMdEntryPx,
  kNotFoundMdEntrySize,
  kNotFoundNoRelatedSym,
  kUnhandledMdEntryTypeValue,
  // my parser
  kNotFoundEqualSignInFieldParseError,
  kNotFoundFieldDelimiterParseError,
  kStringValueIsNotNumberParsingError,
  kConvertingNegativeStringNumberToUnsignedNumber,
};

auto MakeErrorCode(Error code) noexcept -> std::error_code;
auto MakeErrorCode(boost::system::error_code error) noexcept -> std::error_code;

namespace std {

template <>
struct is_error_code_enum<Error> : public true_type {};

}// namespace std
