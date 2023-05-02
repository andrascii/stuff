#pragma once

#include "object.h"
#include "kafka_consumer.h"

namespace message_driven_objects {

using namespace std::chrono;

class Application : public Object {
 public:
  Application();
  ~Application();

  static std::error_code Exec();

 private:
  bool OnTextMessage(const TextMessage& message) override;

 private:
  time_point<system_clock, microseconds> start_;
  uint64_t counter_;
};

}