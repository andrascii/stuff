#include "dispatcher.h"
#include "logger.h"
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
    static auto millions = 0ul;

    ++ctr_;

    if ((ctr_ + 1) / 1'000'000'0 > millions) {
      ++millions;

      const auto delta = high_resolution_clock::now() - start;
      const auto msgs_per_sec = (ctr_ + 1) / duration_cast<milliseconds>(delta).count();

      LOG_INFO(
        "Receiver received '{}' messages, delta is '{}', messages per second '{}'",
        ctr_ + 1,
        duration_cast<milliseconds>(delta),
        msgs_per_sec * 1000
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
    LOG_INFO("Sender object started in the thread '{}', generating '{}' messages...", Thread()->Name(), gen_msg_count_);

    const auto msg = std::make_shared<TestMessage>("Hello!", this, receiver_);
    static auto millions = 0ul;
    static const auto start = high_resolution_clock::now();

    for (size_t i = 0; i < gen_msg_count_; ++i) {
      Dispatcher::Dispatch(msg);

      if ((i + 1) / 1'000'000'0 > millions) {
        ++millions;

        const auto delta = high_resolution_clock::now() - start;
        const auto msgs_per_sec = gen_msg_count_ / duration_cast<milliseconds>(delta).count();

        LOG_INFO(
          "Sender sent '{}' messages, delta is '{}', messages per second '{}'",
          i + 1,
          duration_cast<milliseconds>(delta),
          msgs_per_sec * 1000
        );
      }
    }
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
