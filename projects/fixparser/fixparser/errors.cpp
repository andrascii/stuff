#include "errors.h"

#include <fmt/format.h>

#include <vector>

#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4101)
#include <Windows.h>
using ssize_t = SSIZE_T;
#endif

#include <hffix.hpp>

#if defined(_WIN32)
#pragma warning(pop)
#endif

#include <format>
#include <string_view>

using namespace std::literals;

namespace {

class ErrorCategory final : public std::error_category {
 public:
  [[nodiscard]] const char* name() const noexcept override {
    return "ErrorCategory";
  }

  [[nodiscard]] std::string message(int code) const override {
    switch (static_cast<Error>(code)) {
      case Error::kReaderIsNotComplete: {
        return "reader is not complete";
      }
      case Error::kReaderIsNotValid: {
        return "reader is in invalid state";
      }
      case Error::kUnhandledMessageType: {
        return "unhandled message type by parser";
      }
      case Error::kNotFoundMsgType: {
        return std::format("not found MsgType <{}> field", (int) hffix::tag::MsgType);
      }
      case Error::kNotFoundSymbol: {
        return std::format("not found Symbol <{}> field", (int) hffix::tag::Symbol);
      }
      case Error::kNotFoundAccount: {
        return std::format("not found Account <{}> field", (int) hffix::tag::Account);
      }
      case Error::kNotFoundSenderCompId: {
        return std::format("not found SenderCompID <{}> field", (int) hffix::tag::SenderCompID);
      }
      case Error::kNotFoundTargetCompId: {
        return std::format("not found TargetCompID <{}> field", (int) hffix::tag::TargetCompID);
      }
      case Error::kNotFoundSecurityType: {
        return std::format("not found SecurityType <{}> field", (int) hffix::tag::SecurityType);
      }
      case Error::kNotFoundSecurityGroup: {
        return std::format("not found SecurityGroup <{}> field", (int) hffix::tag::SecurityGroup);
      }
      case Error::kNotFoundMsgSeqNum: {
        return std::format("not found MsgSeqNum <{}> field", (int) (int) hffix::tag::MsgSeqNum);
      }
      case Error::kNotFoundSendingTime: {
        return std::format("not found SendingTime <{}> field", (int) hffix::tag::SendingTime);
      }
      case Error::kNotFoundLastUpdateTime: {
        return std::format("not found LastUpdateTime <{}> field", (int) hffix::tag::LastUpdateTime);
      }
      case Error::kNotFoundNoMdEntries: {
        return std::format("not found NoMDEntries <{}> field", (int) hffix::tag::NoMDEntries);
      }
      case Error::kNotFoundTenor: {
        return "not found Tenor <6215> field";
      }
      case Error::kNotFoundSettlementDate: {
        return std::format("not found SettlementDate <{}> field", (int) hffix::tag::SettlDate);
      }
      case Error::kNotFoundMdReqId: {
        return std::format("not found MDReqID <{}> field", (int) hffix::tag::MDReqID);
      }
      case Error::kNotFoundSubscriptionRequestType: {
        return std::format("not found SubscriptionRequestType <{}> field", (int) hffix::tag::SubscriptionRequestType);
      }
      case Error::kNotFoundMdEntryType: {
        return std::format("not found MDEntryType <{}> field", (int) hffix::tag::MDEntryType);
      }
      case Error::kNotFoundHeartbeatInterval: {
        return std::format("not found HeartBtInt <{}> field", (int) hffix::tag::HeartBtInt);
      }
      case Error::kNotFoundNorTenorNorSettlementDate: {
        return std::format(
          "not found not tenor <{}> nor settlement date <{}> fields",
          6215,
          (int) hffix::tag::SettlDate);
      }
      case Error::kNotFoundMdEntryPx: {
        return std::format("not found MDEntryPx <{}>", (int) hffix::tag::MDEntryPx);
      }
      case Error::kNotFoundMdEntrySize: {
        return std::format("not found MDEntryPx <{}>", (int) hffix::tag::MDEntrySize);
      }
      case Error::kUnhandledMdEntryTypeValue: {
        return std::format("unhandled MDEntryType <{}> value", (int) hffix::tag::MDEntryType);
      }
      case Error::kNotFoundEqualSignInFieldParseError: {
        return std::format(
          "parsing FIX message error: not found equal sign in field");
      }
      case Error::kNotFoundFieldDelimiterParseError: {
        return std::format("parsing FIX message error: not found SOH");
      }
      case Error::kStringValueIsNotNumberParsingError: {
        return std::format(
          "parsing number from string error: value must "
          "consist only from digits");
      }
      case Error::kConvertingNegativeStringNumberToUnsignedNumber: {
        return std::format(
          "parsing number from string error: string contains negative "
          "number but target variable is unsigned type");
      }
      default: {
        return "undefined error code";
      }
    }
  }
};

}// namespace

std::error_code MakeErrorCode(Error code) noexcept {
  static auto category = ErrorCategory{};
  return std::error_code{static_cast<int>(code), category};
}

std::error_code MakeErrorCode(boost::system::error_code error) noexcept {
  class CategoryAdapter final : public std::error_category {
   public:
    explicit CategoryAdapter(const boost::system::error_category& category)
        : category_(category) {}

    [[nodiscard]] const char* name() const noexcept override {
      return category_.name();
    }
    [[nodiscard]] std::string message(int code) const override {
      return category_.message(code);
    }

   private:
    const boost::system::error_category& category_;
  };

  static thread_local std::unordered_map<std::string, CategoryAdapter>
    name_to_category;
  [[maybe_unused]] const auto& [iterator, is_inserted] =
    name_to_category.emplace(error.category().name(), error.category());
  [[maybe_unused]] const auto& [key, value] = *iterator;
  return std::error_code{error.value(), value};
}
