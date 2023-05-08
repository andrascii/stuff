#include "dispatcher.h"
#include "logger.h"
#include "text_message.h"
#include "thread.h"
#include "timer_service.h"
#include "timer_message.h"
#include "signal.h"

/*
 * do I need to implement building a tree using Objects? For what?
 * */

namespace {

void SigIntHandler(int signal) {
  if (signal == SIGINT) {
    mdo::Dispatcher::Quit();
  }
}

}

using namespace std::chrono;
using namespace mdo;

class Application : public Object {
 public:
  Application(TimerService* service)
      : Object{Dispatcher::Instance().Thread()},
        MySignal{this},
        counter_{},
        service_{service} {
    std::signal(SIGINT, SigIntHandler);
    MySignal.Connect(this, &Application::MySlot);
    start_ = system_clock::now();
  }

  ~Application() {
    auto end = system_clock::now();

    SPDLOG_INFO(
      "received {} messages, elapsed time: {} milliseconds",
      counter_,
      duration_cast<milliseconds>(end - start_).count());
  }

  static std::error_code Exec() {
    return Dispatcher::Instance().Exec();
  }

  void MySlot(int v) {
    SPDLOG_INFO("{}: Application called MySlot for value {}", ToString(std::this_thread::get_id()), v);
  }

  void OnThreadStarted() {
    SPDLOG_INFO("{}: Application called OnThreadStarted", ToString(std::this_thread::get_id()));
  }

  Signal<int> MySignal;

 private:
  bool OnTextMessage(TextMessage& message) override {
    SPDLOG_INFO("{}: received message: {}", ToString(std::this_thread::get_id()), message.Message());

    if (message.Sender()) {
      Dispatcher::Dispatch(std::make_shared<TextMessage>("Hello from Application object", this, message.Sender()));
    }

    ++counter_;
    return true;
  }

  bool OnLoopStarted(LoopStarted&) override {
    service_->AddTimer(this, 1s);
    service_->AddTimer(this, 500ms);

    SPDLOG_INFO("OnLoopStarted called for Application");
    MySignal(42);
    return true;
  }

  bool OnTimerMessage(TimerMessage& message) override {
    SPDLOG_INFO("{}: timer ticked, timer id: {}", ToString(std::this_thread::get_id()), message.Id());
    return true;
  }

 private:
  time_point<system_clock, microseconds> start_;
  uint64_t counter_;
  TimerService* service_;
};

class Producer : public Object {
 public:
  explicit Producer(Object* observer, Object* parent = nullptr)
      : Object{parent},
        observer_{observer} {}

  void MySlot(int v) {
    SPDLOG_INFO("{}: Producer called MySlot for value {}", ToString(std::this_thread::get_id()), v);
  }

  void OnThreadStarted() {
    SPDLOG_INFO("{}: Producer called OnThreadStarted", ToString(std::this_thread::get_id()));
  }

 protected:
  bool OnTextMessage(TextMessage& message) override {
    SPDLOG_INFO("{}: received message: {}", ToString(std::this_thread::get_id()), message.Message());
    //SendMessage();
    std::this_thread::sleep_for(1s);
    return true;
  }

  bool OnLoopStarted(LoopStarted&) override {
    SPDLOG_INFO("OnLoopStarted called for Producer");
    //SendMessage();
    return true;
  }

 private:
  void SendMessage() {
    Dispatcher::Dispatch(std::make_shared<TextMessage>("Hello from Producer", this, observer_));
  }

 private:
  Object* observer_;
};

int main() {
  TimerService service;
  service.Start();

  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::info);

  SPDLOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = std::make_shared<Thread>();
  thread->SetName("BackgroundThread");

  auto* app = new Application(&service);
  app->Thread()->SetName("MainThread");

  auto* producer = new Producer{app, thread.get()};

  thread->Started.Connect(app, &Application::OnThreadStarted);
  thread->Started.Connect(producer, &Producer::OnThreadStarted);
  app->MySignal.Connect(producer, &Producer::MySlot);

  thread->Start();
  const auto error = Application::Exec();

  if (error) {
    SPDLOG_ERROR(error.message());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
