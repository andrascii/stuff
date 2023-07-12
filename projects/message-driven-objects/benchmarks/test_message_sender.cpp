#include "test_message_sender.h"
#include "test_message.h"
#include "dispatcher.h"
#include "thread.h"
#include "logger.h"

namespace benchmarks {

using namespace mdo;
using namespace std::chrono;

TestMessageSender::TestMessageSender(const std::shared_ptr<mdo::Thread>& thread, size_t gen_msg_count, Object* receiver)
  : Object{thread},
    gen_msg_count_{gen_msg_count},
    receiver_{receiver} {
  Thread()->Started.Connect(this, &TestMessageSender::OnThreadStarted);
}

void TestMessageSender::OnThreadStarted() {
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

}