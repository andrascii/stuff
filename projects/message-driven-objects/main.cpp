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

namespace {

void SigIntHandler(int signal) {
  if (signal == SIGINT) {
    message_driven_objects::Dispatcher::Quit();
  }
}

}

using namespace std::chrono;
using namespace message_driven_objects;

class Application : public Object {
 public:
  Application()
      : Object{&Dispatcher::Instance()},
        counter_{} {
    std::signal(SIGINT, SigIntHandler);

    start_ = system_clock::now();
  }

  ~Application() {
    auto end = system_clock::now();
    SPDLOG_INFO("received {} messages, elapsed time: {} milliseconds", counter_, duration_cast<milliseconds>(end - start_).count());
  }

  static std::error_code Exec() {
    return Dispatcher::Instance().Exec();
  }

 private:
  bool OnTextMessage(const TextMessage& message) override {
    SPDLOG_INFO("{}: received message: {}", Thread()->Name(), message.Message());

    if (message.Sender()) {
      Dispatcher::Post(std::make_shared<TextMessage>("Hello from Application object", this, message.Sender()));
    }

    ++counter_;
    return true;
  }

 private:
  time_point<system_clock, microseconds> start_;
  uint64_t counter_;
};

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
