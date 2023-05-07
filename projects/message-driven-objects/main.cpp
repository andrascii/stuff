#include "dispatcher.h"
#include "logger.h"
#include "text_message.h"
#include "thread.h"
#include "signal.h"

/*
 * do I need to implement building a tree using Objects? For what?
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
      : Object{Dispatcher::Instance().Thread()},
        MySignal{this},
        counter_{} {
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
    SPDLOG_INFO("OnLoopStarted called for Application");
    MySignal(42);
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

  void MySlot(int v) {
    SPDLOG_INFO("{}: Producer called MySlot for value {}", ToString(std::this_thread::get_id()), v);
  }

  void OnThreadStarted() {
    SPDLOG_INFO("{}: Producer called OnThreadStarted", ToString(std::this_thread::get_id()));
  }

 protected:
  bool OnTextMessage(TextMessage& message) override {
    SPDLOG_INFO("{}: received message: {}", ToString(std::this_thread::get_id()), message.Message());
    SendMessage();
    std::this_thread::sleep_for(1s);
    return true;
  }

  bool OnLoopStarted(LoopStarted&) override {
    SPDLOG_INFO("OnLoopStarted called for Producer");
    SendMessage();
    return true;
  }

 private:
  void SendMessage() {
    Dispatcher::Dispatch(std::make_shared<TextMessage>("Hello from Producer", this, observer_));
  }

 private:
  Object* observer_;
};

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>

int kq;

struct context {
  void (*handler)(struct context *obj);
};

void timer_handler(struct context *obj)
{
  static int n;
  printf("Received timer event via kqueue: %d\n", n++);
}

int main() {
  kq = kqueue();
  assert(kq != -1);

  struct context obj = {};
  obj.handler = timer_handler;

  // start system timer
  int period_ms = 1000;
  struct kevent events[1];
  EV_SET(&events[0], 1234, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, period_ms, &obj);
  assert(0 == kevent(kq, events, 1, NULL, 0, NULL));

  for (;;) {
    //struct timespec *timeout = NULL; // wait indefinitely

    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = 0;

    int n = kevent(kq, NULL, 0, events, 1, &timeout);

    if (n <= 0) {
      continue;
    }

    assert(n > 0);

    context *o = static_cast<context*>(events[0].udata);
    if (events[0].filter == EVFILT_TIMER)
      o->handler(o); // handle timer event
  }

  close(kq);

  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::info);

  SPDLOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = std::make_shared<Thread>();
  thread->SetName("BackgroundThread");

  auto* app = new Application;
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
