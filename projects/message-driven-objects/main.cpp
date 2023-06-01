#include "dispatcher.h"
#include "logger.h"
#include "thread.h"
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
  Application()
      : MySignal{this} {
    std::signal(SIGINT, SigIntHandler);

    MySignal.Connect(this, &Application::MySlot);
    Thread()->Started.Connect(this, &Application::OnThreadStarted);
  }

  void MySlot(int v) {
    SPDLOG_INFO("{}: Application called MySlot for value {}", ToString(std::this_thread::get_id()), v);
  }

  void OnThreadStarted() {
    SPDLOG_INFO("{}: Application called OnThreadStarted", ToString(std::this_thread::get_id()));
    StartTimer(3s);
    MySignal(42);
  }

  Signal<int> MySignal;

 protected:
  bool OnTimerMessage(TimerMessage& message) override {
    SPDLOG_INFO("{}: Application timer ticked, timer id: {}", ToString(std::this_thread::get_id()), message.Id());
    return true;
  }
};

class Producer : public Object {
 public:
  explicit Producer(mdo::Thread* thread)
      : Object{thread} {
    Thread()->Started.Connect(this, &Producer::OnThreadStarted);
  }

  void MySlot(int v) {
    SPDLOG_INFO("{}: Producer called MySlot for value {}", ToString(std::this_thread::get_id()), v);
  }

  void OnThreadStarted() {
    SPDLOG_INFO("{}: Producer called OnThreadStarted", ToString(std::this_thread::get_id()));
    StartTimer(2s);
  }

 protected:
  bool OnTimerMessage(TimerMessage& message) override {
    SPDLOG_INFO("{}: Producer timer ticked, timer id: {}", ToString(std::this_thread::get_id()), message.Id());
    return true;
  }
};

class TimerTest : public Object {
 public:
  TimerTest(mdo::Thread* thread) : Object{thread} {
    StartTimer(1s);
  }

 protected:
  bool OnTimerMessage(TimerMessage& message) override {
    SPDLOG_INFO("{}: TimerTest timer ticked, timer id: {}", ToString(std::this_thread::get_id()), message.Id());
    return true;
  }
};

int main() {
  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::info);

  SPDLOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = std::make_shared<Thread>();
  thread->SetName("BackgroundThread");

  const auto app = std::make_shared<Application>();
  const auto producer = std::make_shared<Producer>(thread.get());

  app->MySignal.Connect(producer.get(), &Producer::MySlot);
  thread->Start();

  const auto error = Dispatcher::Instance().Exec();

  if (error) {
    SPDLOG_ERROR(error.message());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
