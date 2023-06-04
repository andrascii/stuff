#pragma once

namespace mdo {

class KafkaConsumer {
 public:
  KafkaConsumer(const std::string& broker_list, const std::string& topic);

  [[nodiscard]] cppkafka::Message Poll(const std::chrono::milliseconds& timeout);

 private:
  static cppkafka::Configuration CreateConfiguration(const std::string& broker_list) noexcept;

 private:
  cppkafka::Consumer consumer_;
};

}