#include "application.h"
#include "dispatcher.h"
#include "kafka_message_notification.h"
#include "delivery_message.h"
#include "kafka_publisher.h"
#include "logger.h"
#include "thread.h"

using namespace eo;

class Producer : public Object {
 public:
  Producer(Object* observer, const std::string& broker_list, const std::string& topic)
      : observer_{observer},
        publisher_{broker_list, topic} {
    SendKafkaMessage();
  }

 protected:
  bool OnDeliveryMessage(const DeliveryMessage&) override {
    SendKafkaMessage();
    return true;
  }

 private:
  void SendKafkaMessage() {
    publisher_.Publish("Hello, World");
    Dispatcher::Post(std::make_shared<KafkaMessageNotification>(this, observer_));
  }

 private:
  Object* observer_;
  KafkaPublisher publisher_;
};

int main() {
  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::trace);

  SPDLOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  auto* app = new Application;

  auto thread = std::make_shared<Thread>();
  auto producer = std::make_shared<Producer>(app, "localhost:9092", "EO");
  producer->MoveToThread(thread.get());

  thread->Start();
  const auto error = Application::Exec();

  if (error) {
    SPDLOG_ERROR(error.message());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
