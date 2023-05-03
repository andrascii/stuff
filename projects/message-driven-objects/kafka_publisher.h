#pragma once

namespace message_driven_objects {

class KafkaPublisher {
 public:
  KafkaPublisher(const std::string& broker_list, const std::string& topic);

  std::error_code Publish(const std::string& message) noexcept;

 private:
  static cppkafka::Configuration CreateConfiguration(const std::string& broker_list) noexcept;

 private:
  cppkafka::Producer producer_;
  std::string topic_;
};

}