#pragma once

#include "object.h"
#include "kafka_consumer.h"

namespace eo {

using namespace std::chrono;

class Application : public Object {
 public:
  Application();
  ~Application();

  static std::error_code Exec();

 private:
  bool OnKafkaMessageNotification(const KafkaMessageNotification& event) override;

 private:
  KafkaConsumer consumer_;
  time_point<system_clock, microseconds> start_;
  uint64_t counter_;
};

}