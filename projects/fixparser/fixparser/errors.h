#pragma once

#include <system_error>
#include <boost/system/error_code.hpp>

enum class Error {
  kReaderIsNotComplete,
  kReaderIsNotValid,
  kReaderUnhandledMessageType,
  kNotFoundSymbol,
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
};

auto MakeErrorCode(Error code) noexcept -> std::error_code;
auto MakeErrorCode(boost::system::error_code error) noexcept -> std::error_code;

namespace std {

template <>
struct is_error_code_enum<Error> : public true_type {};

}// namespace std
