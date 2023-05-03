#include "kafka_publisher.h"

namespace message_driven_objects {

KafkaPublisher::KafkaPublisher(const std::string& broker_list, const std::string& topic)
    : producer_{CreateConfiguration(broker_list)},
      topic_{topic} {}

std::error_code KafkaPublisher::Publish(const std::string& message) noexcept {
  try {
    producer_.produce(cppkafka::MessageBuilder(topic_).partition(0).payload(message));
    producer_.flush();
    return std::error_code{};
  } catch (const std::exception& ex) {
    return std::make_error_code(std::errc::interrupted);
  }
}

cppkafka::Configuration KafkaPublisher::CreateConfiguration(const std::string& broker_list) noexcept {
  return cppkafka::Configuration{{"metadata.broker.list", broker_list}};
}

}
