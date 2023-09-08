#include "producer.h"

namespace write_to_kafka {

Producer::Producer(const Config& config)
    : producer_{CreateConfiguration(config)},
      config_{config} {}

std::error_code Producer::Write(const std::string& message) noexcept {
  try {
    producer_.produce(cppkafka::MessageBuilder(config_.KafkaTopic()).partition(0).payload(message));
    producer_.flush();
    return std::error_code{};
  } catch (const std::exception& ex) {
    return std::make_error_code(std::errc::interrupted);
  }
}

cppkafka::Configuration Producer::CreateConfiguration(const Config& config) noexcept {
  return cppkafka::Configuration{
    {"metadata.broker.list", config.KafkaBrokerList()}
  };
}

}// namespace write_to_kafka
