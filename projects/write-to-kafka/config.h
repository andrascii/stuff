#pragma once

#include "types.h"

namespace write_to_kafka {

class Config {
 public:
  explicit Config(const std::filesystem::path& path);

  const std::string& KafkaBrokerList() const noexcept;
  const std::string& KafkaTopic() const noexcept;

 private:
  std::string kafka_broker_list_;
  std::string kafka_topic_;
};

}// namespace write_to_kafka