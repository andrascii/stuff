#include "dispatcher.h"
#include "logger.h"
#include "thread.h"
#include "timer_message.h"
#include "test_message_receiver.h"
#include "test_message_sender.h"
#include "music_player.h"
#include "measure.h"
#include "single_thread_execution_policy.h"
#include "thread_pool.h"
#include "thread_pool_execution_policy.h"

/*
 * do I need to implement building a tree using Objects? For what?
 * */

namespace {

void SigIntHandler(int signal) {
  if (signal == SIGINT) {
    mdo::Dispatcher::Quit();
  }
}

constexpr size_t kGenMsgsCount = 100000000;

}// namespace

using namespace mdo;
using namespace benchmarks;
using namespace std::chrono;

auto SendAndReceiveTestMessageBenchmark(uint64_t iterations) {
  LOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = Thread::Create("sender_thread");
  thread->Start();

  const auto policy = std::make_shared<SingleThreadExecutionPolicy>(thread);

  const auto receiver = std::make_shared<TestMessageReceiver>(iterations);
  const auto sender = std::make_shared<TestMessageSender>(policy, iterations, receiver.get());

  const auto result = Dispatcher::Instance().Exec();

  LOG_INFO("{} done", __FUNCTION__);

  return result;
}

auto SignalSendBenchmark(uint64_t iterations) {
  using namespace std::chrono;

  static const auto benchmark_name = __FUNCTION__;

  class SignalReceiver : public Object {
   public:
    SignalReceiver(const std::shared_ptr<IExecutionPolicy>& execution_policy, uint64_t iterations)
        : Object{ execution_policy },
          iterations_{iterations},
          on_volume_changed_benchmark_done_{},
          on_song_changed_benchmark_done_{} {}

    void OnVolumeChanged(uint64_t) {
      constexpr size_t kBatchSize = 1'000'000'0;
      static auto batches = 0ul;

      measure_.IncrementCalls();

      const auto call_count = measure_.CallCount();
      const auto current_batch = measure_.CallCount() / kBatchSize;

      if (current_batch > batches) {
        ++batches;

        const auto metrics = measure_.GetMetrics();

        LOG_INFO(
          "volume changed notifications '{}', notifications per second '{}', "
          "time avg='{}', time min='{}', time max='{}', median='{}'",
          metrics.call_count,
          metrics.avg_call_count,
          metrics.time_avg,
          metrics.time_min,
          duration_cast<milliseconds>(metrics.time_max),
          duration_cast<milliseconds>(metrics.time_median)
        );
      }

      if (call_count == iterations_) {
        measure_.Reset();

        on_volume_changed_benchmark_done_ = true;

        if (on_song_changed_benchmark_done_) {
          LOG_INFO("{} done", benchmark_name);
          Dispatcher::Quit();
        }
      }
    }

    void OnSongChanged(const MusicPlayer::Song&) {
      constexpr size_t kBatchSize = 1'000'000'0;
      static auto batches = 0ul;

      measure_.IncrementCalls();

      const auto call_count = measure_.CallCount();
      const auto current_batch = measure_.CallCount() / kBatchSize;

      if (current_batch > batches) {
        ++batches;

        const auto metrics = measure_.GetMetrics();

        LOG_INFO(
          "song changed notifications '{}', notifications per second '{}', "
          "time avg='{}', time min='{}', time max='{}', median='{}'",
          metrics.call_count,
          metrics.avg_call_count,
          metrics.time_avg,
          metrics.time_min,
          duration_cast<milliseconds>(metrics.time_max),
          duration_cast<milliseconds>(metrics.time_median)
        );
      }

      if (call_count == iterations_) {
        measure_.Reset();

        on_song_changed_benchmark_done_ = true;

        if (on_volume_changed_benchmark_done_) {
          LOG_INFO("{} done", benchmark_name);
          Dispatcher::Quit();
        }
      }
    }
   private:
    uint64_t iterations_;
    bool on_volume_changed_benchmark_done_;
    bool on_song_changed_benchmark_done_;
    Measure measure_;
  };

  LOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = Thread::Create("receiver_thread");
  thread->Start();

  const auto policy = std::make_shared<SingleThreadExecutionPolicy>(thread);

  const auto receiver = std::make_shared<SignalReceiver>(policy, iterations);
  const auto sender = std::make_shared<MusicPlayer>(iterations);

  sender->OnVolumeChanged.Connect(receiver.get(), &SignalReceiver::OnVolumeChanged);
  sender->OnSongChanged.Connect(receiver.get(), &SignalReceiver::OnSongChanged);

  return Dispatcher::Instance().Exec();
}

class A : public Object {
 public:
  explicit A(std::shared_ptr<IExecutionPolicy> policy) : Object{std::move(policy)} {}

 private:
  bool OnTestMessage(TestMessage& message) override {
    LOG_INFO("{}: received test message '{}'", std::this_thread::get_id(), message.Data());
    return true;
  }
};

class B : public Object {
 public:
  B(uint64_t iterations, Object* receiver) : receiver_{receiver}, iterations_{iterations} {
    Thread()->Started.Connect(this, &B::OnStart);
  }

  void OnStart() {
    for (uint64_t i = 0; i < iterations_; ++i) {
      const auto msg = std::make_shared<TestMessage>(std::to_string(i), this, receiver_);
      Dispatcher::Dispatch(msg);
      LOG_INFO("sent {} message", i);
    }
  }

 private:
  Object* receiver_;
  uint64_t iterations_;
};

int main() {
  std::signal(SIGINT, SigIntHandler);
  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::info);

  /*const auto pool = std::make_shared<ThreadPool>(10);
  pool->Start();

  const auto policy = std::make_shared<ThreadPoolExecutionPolicy>(pool);
  const auto a = std::make_shared<A>(policy);
  const auto b = std::make_shared<B>(10, a.get());

  Dispatcher::Instance().Exec();

  (void)kGenMsgsCount;*/

  std::vector<std::function<std::error_code()>> benchmarks;

  benchmarks.emplace_back([] {
    return SendAndReceiveTestMessageBenchmark(kGenMsgsCount);
  });

  benchmarks.emplace_back([] {
    return SignalSendBenchmark(kGenMsgsCount);
  });

  for (const auto& benchmark : benchmarks) {
    const auto error = benchmark();

    if (error) {
      LOG_ERROR(error.message());
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
