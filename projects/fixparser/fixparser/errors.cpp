#include "errors.h"

#include <fmt/format.h>
#include <vector>

#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4101)
#include <Windows.h>
using ssize_t = SSIZE_T;
#endif

#include <hffix.hpp>

#if defined(_WIN32)
#pragma warning(pop)
#endif

#include <string_view>

using namespace std::literals;

namespace {

class ErrorCategory final : public std::error_category {
 public:
  [[nodiscard]] const char* name() const noexcept override { return "ErrorCategory"; }

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
        return fmt::format("not found MsgType <{}> field", hffix::tag::MsgType);
      }
      case Error::kNotFoundSymbol: {
        return fmt::format("not found Symbol <{}> field", hffix::tag::Symbol);
      }
      case Error::kNotFoundAccount: {
        return fmt::format("not found Account <{}> field", hffix::tag::Account);
      }
      case Error::kNotFoundSenderCompId: {
        return fmt::format("not found SenderCompID <{}> field", hffix::tag::SenderCompID);
      }
      case Error::kNotFoundTargetCompId: {
        return fmt::format("not found TargetCompID <{}> field", hffix::tag::TargetCompID);
      }
      case Error::kNotFoundSecurityType: {
        return fmt::format("not found SecurityType <{}> field", hffix::tag::SecurityType);
      }
      case Error::kNotFoundSecurityGroup: {
        return fmt::format("not found SecurityGroup <{}> field", hffix::tag::SecurityGroup);
      }
      case Error::kNotFoundMsgSeqNum: {
        return fmt::format("not found MsgSeqNum <{}> field", hffix::tag::MsgSeqNum);
      }
      case Error::kNotFoundSendingTime: {
        return fmt::format("not found SendingTime <{}> field", hffix::tag::SendingTime);
      }
      case Error::kNotFoundLastUpdateTime: {
        return fmt::format("not found LastUpdateTime <{}> field", hffix::tag::LastUpdateTime);
      }
      case Error::kNotFoundNoMdEntries: {
        return fmt::format("not found NoMDEntries <{}> field", hffix::tag::NoMDEntries);
      }
      case Error::kNotFoundTenor: {
        return "not found Tenor <6215> field";
      }
      case Error::kNotFoundSettlementDate: {
        return fmt::format("not found SettlementDate <{}> field", hffix::tag::SettlDate);
      }
      case Error::kNotFoundMdReqId: {
        return fmt::format("not found MDReqID <{}> field", hffix::tag::MDReqID);
      }
      case Error::kNotFoundSubscriptionRequestType: {
        return fmt::format("not found SubscriptionRequestType <{}> field", hffix::tag::SubscriptionRequestType);
      }
      case Error::kNotFoundMdEntryType: {
        return fmt::format("not found MDEntryType <{}> field", hffix::tag::MDEntryType);
      }
      case Error::kNotFoundHeartbeatInterval: {
        return fmt::format("not found HeartBtInt <{}> field", hffix::tag::HeartBtInt);
      }
      case Error::kNotFoundNorTenorNorSettlementDate: {
        return fmt::format("not found not tenor <{}> nor settlement date <{}> fields", 6215, hffix::tag::SettlDate);
      }
      case Error::kNotFoundMdEntryPx: {
        return fmt::format("not found MDEntryPx <{}>", hffix::tag::MDEntryPx);
      }
      case Error::kNotFoundMdEntrySize: {
        return fmt::format("not found MDEntryPx <{}>", hffix::tag::MDEntrySize);
      }
      case Error::kUnhandledMdEntryTypeValue: {
        return fmt::format("unhandled MDEntryType <{}> value", hffix::tag::MDEntryType);
      }
      case Error::kNotFoundEqualSignInFieldParseError: {
        return fmt::format("parsing FIX message error: not found equal sign in field");
      }
      case Error::kNotFoundFieldDelimiterParseError: {
        return fmt::format("parsing FIX message error: not found SOH");
      }
      case Error::kStringValueIsNotNumberParsingError: {
        return fmt::format("parsing number from string error: value must consist only from digits");
      }
      case Error::kConvertingNegativeStringNumberToUnsignedNumber: {
        return fmt::format("parsing number from string error: string contains negative number but target variable is unsigned type");
      }
      default: {
        return "undefined error code";
      }
    }
  }
};

}// namespace

auto MakeErrorCode(Error code) noexcept -> std::error_code {
  static auto category = ErrorCategory{};
  return std::error_code{static_cast<int>(code), category};
}

auto MakeErrorCode(boost::system::error_code error) noexcept -> std::error_code {
  class CategoryAdapter final : public std::error_category {
   public:
    explicit CategoryAdapter(const boost::system::error_category& category) : category_(category) {}

    [[nodiscard]] const char* name() const noexcept override { return category_.name(); }
    [[nodiscard]] std::string message(int code) const override { return category_.message(code); }

   private:
    const boost::system::error_category& category_;
  };

  static thread_local std::unordered_map<std::string, CategoryAdapter> name_to_category;
  [[maybe_unused]] const auto& [iterator, is_inserted] = name_to_category.emplace(error.category().name(), error.category());
  [[maybe_unused]] const auto& [key, value] = *iterator;
  return std::error_code{error.value(), value};
}