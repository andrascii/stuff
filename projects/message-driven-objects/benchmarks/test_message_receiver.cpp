#include "test_message_receiver.h"
#include "thread.h"
#include "logger.h"

namespace benchmarks {

using namespace mdo;
using namespace std::chrono;

TestMessageReceiver::TestMessageReceiver(uint64_t iterations) : ctr_{}, iterations_{iterations} {
  Thread()->Started.Connect(this, &TestMessageReceiver::OnThreadStarted);
}

TestMessageReceiver::~TestMessageReceiver() {
  LOG_INFO("TestMessageReceiver destroyed in thread '{}'", current_thread_data->Thread()->Name());
}

void TestMessageReceiver::OnThreadStarted() {
  LOG_INFO("TestMessageReceiver object started in the thread '{}'", Thread()->Name());
}

bool TestMessageReceiver::OnTestMessage(TestMessage&) {
  constexpr size_t kBatchSize = 1'000'000'0;
  static auto batches = 0ul;

  measure_.IncrementCalls();

  const auto call_count = measure_.CallCount();
  const auto current_batch = measure_.CallCount() / kBatchSize;

  if (current_batch > batches) {
    ++batches;

    const auto metrics = measure_.GetMetrics();

    LOG_INFO(
      "got '{}' messages, per second '{}', "
      "avg='{}', min='{}', max='{}', median='{}'",
      call_count,
      metrics.avg_call_count,
      metrics.time_avg,
      metrics.time_min,
      duration_cast<milliseconds>(metrics.time_max),
      duration_cast<milliseconds>(metrics.time_median)
    );
  }

  if (call_count == iterations_) {
    Dispatcher::Quit();
  }

  return true;
}

}