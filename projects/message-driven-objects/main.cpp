#include "application.h"
#include "dispatcher.h"
#include "logger.h"
#include "text_message.h"
#include "thread.h"

/*
 *
 * 1. Implement signals/slots
 * 2. do I need to implement building a tree using Objects? For what?
 *
 * */

using namespace message_driven_objects;

class Producer : public Object {
 public:
  explicit Producer(Object* observer, Object* parent = nullptr)
      : Object{parent},
        observer_{observer} {}

 protected:
  bool OnTextMessage(const TextMessage& message) override {
    SPDLOG_INFO("{}: received message: {}", Thread()->Name(), message.Message());
    SendMessage();
    std::this_thread::sleep_for(400ms);
    return true;
  }

  bool OnLoopStarted(const LoopStarted&) override {
    SPDLOG_INFO("OnLoopStarted called for Producer");
    SendMessage();
    return true;
  }

 private:
  void SendMessage() {
    Dispatcher::Post(std::make_shared<TextMessage>("Hello from Producer", this, observer_));
  }

 private:
  Object* observer_;
};

int main() {
  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::trace);

  SPDLOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = std::make_shared<Thread>();
  thread->SetName("BackgroundThread");
  thread->Start();

  auto* app = new Application;
  app->Thread()->SetName("MainThread");

  new Producer{app, thread.get()};

  const auto error = Application::Exec();

  if (error) {
    SPDLOG_ERROR(error.message());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
