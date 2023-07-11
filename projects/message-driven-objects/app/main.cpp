#include "dispatcher.h"
#include "logger.h"
#include "signal_impl.h"
#include "thread.h"
#include "timer_message.h"
#include "test_message.h"

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

constexpr size_t kGenMsgsCount = 100000000;

class Receiver : public Object {
 public:
   Receiver() : ctr_{} {
    Thread()->Started.Connect(this, &Receiver::OnThreadStarted);
  }

  void OnThreadStarted() {
    LOG_INFO("Receiver object started in the thread '{}'", Thread()->Name());
  }

  size_t Counter() const noexcept {
    return ctr_;
  }

 protected:
  bool OnTestMessage(TestMessage&) override {
    static const auto start = high_resolution_clock::now();

    ++ctr_;

    if (ctr_ == kGenMsgsCount - 1) {
      const auto delta = high_resolution_clock::now() - start;
      const auto msgs_per_sec = (ctr_ + 1) / duration_cast<milliseconds>(delta).count();

      LOG_INFO(
        "Receiver done, current value is '{}', delta is '{}', messages per millisecond is '{}'",
        ctr_ + 1,
        duration_cast<milliseconds>(delta),
        msgs_per_sec
      );
    }

    return true;
  }

private:
  size_t ctr_;
};

class Sender : public Object {
 public:
  explicit Sender(const std::shared_ptr<mdo::Thread>& thread, size_t gen_msg_count, Object* receiver)
    : Object{ thread },
      gen_msg_count_{ gen_msg_count },
      receiver_{ receiver } {
    Thread()->Started.Connect(this, &Sender::OnThreadStarted);
  }

  void OnThreadStarted() {
    LOG_INFO("Sender object started in the thread '{}', generating messages...", Thread()->Name());

    static const auto start = high_resolution_clock::now();

    for (size_t i = 0; i < gen_msg_count_; ++i) {
      Dispatcher::Dispatch(std::make_shared<TestMessage>("Hello!", this, receiver_));
    }

    const auto delta = high_resolution_clock::now() - start;
    const auto msgs_per_sec = gen_msg_count_ / duration_cast<milliseconds>(delta).count();

    LOG_INFO(
      "Sender done, delta is '{}', messages per millisecond is '{}'",
      duration_cast<milliseconds>(delta),
      msgs_per_sec
    );
  }

private:
  size_t gen_msg_count_;
  Object* receiver_;
};

int main() {
  std::signal(SIGINT, SigIntHandler);
  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::info);

  LOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = Thread::Create("sender_thread");
  thread->Start();

  const auto receiver = std::make_shared<Receiver>();
  const auto sender = std::make_shared<Sender>(thread, kGenMsgsCount, receiver.get());

  const auto error = Dispatcher::Instance().Exec();

  if (error) {
    LOG_ERROR(error.message());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
