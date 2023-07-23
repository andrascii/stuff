#include "dispatcher.h"
#include "logger.h"
#include "thread.h"
#include "test_message_receiver.h"
#include "test_message_sender.h"
#include "music_player.h"
#include "measure.h"

/*
 * do I need to implement building a tree using Objects? For what?
 * */

namespace {

void SigIntHandler(int signal) {
  if (signal == SIGINT || signal == SIGHUP) {
    mdo::Dispatcher::Quit();
  }
}

[[maybe_unused]] constexpr size_t kGenMsgsCount = 100000000;

}// namespace

using namespace mdo;
using namespace benchmarks;
using namespace std::chrono;

auto SendAndReceiveTestMessageBenchmark(uint64_t iterations) {
  LOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = Thread::Create("sender_thread");

  const auto receiver = std::make_shared<TestMessageReceiver>(iterations);
  const auto sender = std::make_shared<TestMessageSender>(thread, iterations, receiver.get());

  thread->Start();
  const auto result = Dispatcher::Instance().Exec();

  LOG_INFO("{} done", __FUNCTION__);

  return result;
}

auto SignalSendBenchmark(uint64_t iterations) {
  using namespace std::chrono;

  static const auto benchmark_name = __FUNCTION__;

  class SignalReceiver : public Object {
   public:
    SignalReceiver(const std::shared_ptr<mdo::Thread>& thread, uint64_t iterations)
        : Object{thread},
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

  const auto receiver = std::make_shared<SignalReceiver>(thread, iterations);
  const auto sender = std::make_shared<MusicPlayer>(iterations);

  sender->OnVolumeChanged.Connect(receiver.get(), &SignalReceiver::OnVolumeChanged);
  sender->OnSongChanged.Connect(receiver.get(), &SignalReceiver::OnSongChanged);

  return Dispatcher::Instance().Exec();
}

int main() {
  std::signal(SIGINT, SigIntHandler);
  std::signal(SIGHUP, SigIntHandler);
  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::info);

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
