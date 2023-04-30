#include "application.h"
#include "dispatcher.h"
#include "kafka_message_notification.h"
#include "delivery_message.h"

namespace {

void SigIntHandler(int signal) {
  if (signal == SIGINT) {
    eo::Dispatcher::Quit();
  }
}

}

namespace eo {

Application::Application()
    : Object{&Dispatcher::Instance()},
      consumer_{"localhost:9092", "EO"},
      counter_{} {
  std::signal(SIGINT, SigIntHandler);

  start_ = system_clock::now();
}

Application::~Application() {
  auto end = system_clock::now();
  SPDLOG_INFO("received {} messages, elapsed time: {} milliseconds", counter_, duration_cast<milliseconds>(end - start_).count());
}

std::error_code Application::Exec() {
  return Dispatcher::Instance().Exec();
}

bool Application::OnKafkaMessageNotification(const KafkaMessageNotification& message) {
  cppkafka::Message msg = consumer_.Poll(1s);

  if (!msg) {
    SPDLOG_INFO("topic is empty...");
    return false;
  }

  const std::string payload = msg.get_payload();
  SPDLOG_INFO("received message from kafka: {}", payload);

  if (message.Sender()) {
    Dispatcher::Post(std::make_shared<DeliveryMessage>(this, message.Sender()));
  }

  ++counter_;
  return true;
}

}