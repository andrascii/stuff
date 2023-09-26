#include "config.h"

namespace {

void ThrowIfFieldDoesNotExists(const nlohmann::json& json, const std::string& field_name) {
  if (!json.contains(field_name)) {
    throw std::logic_error{fmt::format("not found '{}' field in the json", field_name)};
  }
}

template <typename T>
void ThrowIfInvalidType(const nlohmann::json& json, const std::string& field_name) {
  using PureType = std::decay_t<T>;

  if constexpr (std::is_same_v<PureType, bool>) {
    if (!json[field_name].is_boolean()) {
      throw std::logic_error{fmt::format("field '{}' is not a boolean", field_name)};
    }
  } else if constexpr (std::is_same_v<PureType, std::string>) {
    if (!json[field_name].is_string()) {
      throw std::logic_error{fmt::format("field '{}' is not a string type", field_name)};
    }
  } else {
    throw std::logic_error{"add overload for T"};
  }
}

std::string ReadRequiredStringOption(const nlohmann::json& json, const std::string& field_name) {
  ThrowIfFieldDoesNotExists(json, field_name);
  ThrowIfInvalidType<std::string>(json, field_name);
  return json[field_name];
}

bool ReadRequiredBoolOption(const nlohmann::json& json, const std::string& field_name) {
  ThrowIfFieldDoesNotExists(json, field_name);
  ThrowIfInvalidType<bool>(json, field_name);
  return json[field_name];
}

}// namespace

namespace write_to_kafka {

Config::Config(const std::filesystem::path& path)
    : log_level_{spdlog::level::info},
      quiet_mode_{} {
  const mio::mmap_source mmap{path.generic_string()};
  const std::string_view view{mmap.data(), mmap.size()};

  SPDLOG_INFO(view);

  const nlohmann::json json = nlohmann::json::parse(view);
  kafka_broker_list_ = ReadRequiredStringOption(json, "kafka_broker_list");
  kafka_topic_ = ReadRequiredStringOption(json, "kafka_topic");

  const auto log_level = ReadRequiredStringOption(json, "log_level");
  log_level_ = spdlog::level::from_str(log_level);

  if (log_level_ == spdlog::level::off) {
    throw std::logic_error{
      fmt::format("incorrect value of 'log_level' field in the json which is: {}", log_level)};
  }

  quiet_mode_ = ReadRequiredBoolOption(json, "quiet_mode");
}

const std::string& Config::KafkaBrokerList() const noexcept {
  return kafka_broker_list_;
}

const std::string& Config::KafkaTopic() const noexcept {
  return kafka_topic_;
}

spdlog::level::level_enum Config::LogLevel() const noexcept {
  return log_level_;
}

bool Config::QuietMode() const noexcept {
  return quiet_mode_;
}

}// namespace write_to_kafka