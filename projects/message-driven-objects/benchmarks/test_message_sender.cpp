#include "test_message_sender.h"
#include "test_message.h"
#include "dispatcher.h"
#include "thread.h"
#include "logger.h"

namespace benchmarks {

using namespace mdo;
using namespace std::chrono;

TestMessageSender::TestMessageSender(const std::shared_ptr<IExecutionPolicy>& execution_policy, size_t gen_msg_count, Object* receiver)
  : Object{execution_policy},
    gen_msg_count_{gen_msg_count},
    receiver_{receiver} {
  Thread()->Started.Connect(this, &TestMessageSender::OnThreadStarted);
}

void TestMessageSender::OnThreadStarted() {
  LOG_INFO("TestMessageSender object started in the thread '{}', generating '{}' messages...", Thread()->Name(), gen_msg_count_);

  const auto msg = std::make_shared<TestMessage>("Hello!", this, receiver_);

  constexpr size_t kBatchSize = 1'000'000'0;
  static auto batches = 0ul;

  for (size_t i = 0; i < gen_msg_count_; ++i) {
    measure_.IncrementCalls();

    Dispatcher::Dispatch(msg);

    const auto call_count = measure_.CallCount();
    const auto current_batch = measure_.CallCount() / kBatchSize;

    if (current_batch > batches) {
      ++batches;

      const auto metrics = measure_.GetMetrics();

      LOG_INFO(
        "sent '{}' messages, messages per second '{}', "
        "time avg='{}', time min='{}', time max='{}', median='{}'",
        call_count,
        metrics.avg_call_count,
        metrics.time_avg,
        metrics.time_min,
        duration_cast<milliseconds>(metrics.time_max),
        duration_cast<milliseconds>(metrics.time_median)
      );
    }
  }
}

}