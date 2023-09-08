#pragma once

#include "config.h"

namespace write_to_kafka {

class Producer final {
 public:
  Producer(const Config& config);

  std::error_code Write(const std::string& message) noexcept;

 private:
  static cppkafka::Configuration CreateConfiguration(const Config& config) noexcept;

 private:
  cppkafka::Producer producer_;
  Config config_;
};

}// namespace write_to_kafka