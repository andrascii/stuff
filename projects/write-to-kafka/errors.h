#pragma once

namespace write_to_kafka {

enum class Error {
  kCommandLineParsingError = 1,
  kConfigParsingError,
};

std::error_code MakeErrorCode(Error code) noexcept;
std::error_code MakeErrorCode(boost::system::error_code error) noexcept;

}// namespace write_to_kafka

namespace std {

template <>
struct is_error_code_enum<write_to_kafka::Error> : public true_type {};

}// namespace std
