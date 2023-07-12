#include "dispatcher.h"
#include "logger.h"
#include "thread.h"
#include "timer_message.h"
#include "test_message.h"
#include "test_message_receiver.h"
#include "test_message_sender.h"
#include "music_player.h"

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

auto SendAndReceiveTestMessageBenchmark() {
  LOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = Thread::Create("sender_thread");
  thread->Start();

  const auto receiver = std::make_shared<TestMessageReceiver>();
  const auto sender = std::make_shared<TestMessageSender>(thread, kGenMsgsCount, receiver.get());

  return Dispatcher::Instance().Exec();
}

auto SignalSendBenchmark() {
  class Receiver : public Object {
  public:
    Receiver(const std::shared_ptr<mdo::Thread>& thread) : Object{ thread } {}

    void OnVolumeChanged(uint64_t) {
      static const auto start = high_resolution_clock::now();
      static auto millions = 0ul;
      static size_t ctr = 0;

      ++ctr;

      if ((ctr + 1) / 1'000'000'0 > millions) {
        ++millions;

        const auto delta = high_resolution_clock::now() - start;
        const auto signals_per_sec = (ctr + 1) / duration_cast<milliseconds>(delta).count();

        LOG_INFO(
          "Receiver received '{}' notifications about volume changed, delta is '{}', notifications per second '{}'",
          ctr + 1,
          duration_cast<milliseconds>(delta),
          signals_per_sec * 1000
        );
      }
    }

    void OnSongChanged(const MusicPlayer::Song&) {
      static const auto start = high_resolution_clock::now();
      static auto millions = 0ul;
      static size_t ctr = 0;

      ++ctr;

      if ((ctr + 1) / 1'000'000'0 > millions) {
        ++millions;

        const auto delta = high_resolution_clock::now() - start;
        const auto signals_per_sec = (ctr + 1) / duration_cast<milliseconds>(delta).count();

        LOG_INFO(
          "Receiver received '{}' notifications about song changed, delta is '{}', notifications per second '{}'",
          ctr + 1,
          duration_cast<milliseconds>(delta),
          signals_per_sec * 1000
        );
      }
    }
  };

  LOG_INFO("the main thread id: {}", ToString(std::this_thread::get_id()));

  const auto thread = Thread::Create("receiver_thread");
  thread->Start();

  const auto receiver = std::make_shared<Receiver>(thread);
  const auto sender = std::make_shared<MusicPlayer>();

  sender->OnVolumeChanged.Connect(receiver.get(), &Receiver::OnVolumeChanged);
  sender->OnSongChanged.Connect(receiver.get(), &Receiver::OnSongChanged);

  return Dispatcher::Instance().Exec();
}

int main() {
  std::signal(SIGINT, SigIntHandler);
  EnableConsoleLogging();
  Logger()->set_level(spdlog::level::info);

  auto error = SignalSendBenchmark();

  if (error) {
    LOG_ERROR(error.message());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
