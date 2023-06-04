#include "kafka_consumer.h"

namespace mdo {

KafkaConsumer::KafkaConsumer(const std::string& broker_list, const std::string& topic)
    : consumer_{CreateConfiguration(broker_list)} {
  consumer_.set_assignment_callback([](const cppkafka::TopicPartitionList& partitions) {
    std::cout << "Got assigned: " << partitions << std::endl;
  });

  consumer_.set_revocation_callback([](const cppkafka::TopicPartitionList& partitions) {
    std::cout << "Got revoked: " << partitions << std::endl;
  });

  consumer_.subscribe({ topic });
}

cppkafka::Message KafkaConsumer::Poll(const std::chrono::milliseconds& timeout) {
  return consumer_.poll(timeout);
}

cppkafka::Configuration KafkaConsumer::CreateConfiguration(const std::string& broker_list) noexcept {
  return cppkafka::Configuration{
    { "metadata.broker.list", broker_list },
    { "group.id", "group" },
    { "enable.auto.commit", true }
  };
}

}