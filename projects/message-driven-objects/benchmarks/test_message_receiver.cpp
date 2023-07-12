#include "test_message_receiver.h"
#include "thread.h"
#include "logger.h"

namespace benchmarks {

using namespace mdo;
using namespace std::chrono;

constexpr size_t kGenMsgsCount = 100000000;

TestMessageReceiver::TestMessageReceiver() : ctr_{} {
  Thread()->Started.Connect(this, &TestMessageReceiver::OnThreadStarted);
}

void TestMessageReceiver::OnThreadStarted() {
  LOG_INFO("Receiver object started in the thread '{}'", Thread()->Name());
}

size_t TestMessageReceiver::Counter() const noexcept {
  return ctr_;
}

bool TestMessageReceiver::OnTestMessage(TestMessage&) {
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

}