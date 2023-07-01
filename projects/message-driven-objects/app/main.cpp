#include "dispatcher.h"
#include "logger.h"
#include "signal_impl.h"
#include "thread.h"
#include "timer_message.h"

/*
 * do I need to implement building a tree using Objects? For what?
 * */

namespace {

void SigIntHandler(int signal) {
  if (signal == SIGINT) {
    mdo::Dispatcher::Quit();
  }
}

}// namespace

using namespace std::chrono;
using namespace mdo;

class Client1 : public Object {
 public:
  Client1()
      : SendMessageSignal{this} {
    std::signal(SIGINT, SigIntHandler);

    SendMessageSignal.Connect(this, &Client1::OnMessage);
    Thread()->Started.Connect(this, &Client1::OnThreadStarted);
  }

  void OnMessage(const std::string& message) {
    SPDLOG_INFO("{}: Client1 called OnMessage for value '{}'", Thread::Tid(), message);
  }

  void OnThreadStarted() {
    SPDLOG_INFO("{}: Client1 called OnThreadStarted", Thread::Tid());
    StartTimer(3s);
    SendMessageSignal("Hello, World!");
  }

  Signal<const std::string&> SendMessageSignal;

 protected:
  bool OnTimerMessage(TimerMessage& message) override {
    SPDLOG_INFO("{}: Client1 timer ticked, timer id: {}", Thread::Tid(), message.Id());
    return true;
  }
};

class Client2 : public Object {
 public:
  explicit Client2(mdo::Thread* thread)
      : Object{thread} {
    Thread()->Started.Connect(this, &Client2::OnThreadStarted);
  }

  void OnMessage(const std::string& message) {
    SPDLOG_INFO("{}: Client2 called OnMessage for value {}", Thread::Tid(), message);
  }

  void OnThreadStarted() {
    SPDLOG_INFO("{}: Client2 called OnThreadStarted", Thread::Tid());
    StartTimer(2s);
  }

 protected:
  bool OnTimerMessage(TimerMessage& message) override {
    SPDLOG_INFO("{}: Client2 timer ticked, timer id: {}", Thread::Tid(), message.Id());
    return true;
  }
};

std::shared_ptr<Thread> CreateThread(const std::string& name = "") {
  const auto thread = std::make_shared<Thread>();
  thread->SetName(name);
  return thread;
}

int main() {
  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::info);

  SPDLOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));
  Dispatcher::Instance().Thread()->SetName("MainThread");

  const auto thread = CreateThread("BackgroundThread");

  const auto client1 = std::make_shared<Client1>();
  const auto client2 = std::make_shared<Client2>(thread.get());

  thread->Start();

  client1->SendMessageSignal.Connect(client2.get(), &Client2::OnMessage);

  const auto error = Dispatcher::Instance().Exec();

  if (error) {
    SPDLOG_ERROR(error.message());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
