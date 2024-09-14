#include "errors.h"

namespace {

using namespace write_to_kafka;

class ErrorCategory final : public std::error_category {
 public:
  [[nodiscard]] const char* name() const noexcept override {
    return "ErrorCategory";
  }

  [[nodiscard]] std::string message(int code) const override {
    switch (static_cast<Error>(code)) {
      case Error::kCommandLineParsingError: {
        return "command line parsing error";
      }
      case Error::kConfigParsingError: {
        return "config parsing error";
      }
      default: {
        return "undefined error code";
      }
    }
  }
};

}// namespace

namespace write_to_kafka {

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

}// namespace write_to_kafka
