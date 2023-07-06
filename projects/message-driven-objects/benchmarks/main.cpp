#include "object.h"
#include "thread.h"
#include "benchmark_message.h"

using namespace mdo;

void BenchmarkDispatchingMessages(benchmark::State& state) {
  class BenchmarkMessageSender : public Object {
   public:
    BenchmarkMessageSender(Object* receiver)
        : receiver_{receiver} {
      Thread()->Started.Connect(this, &BenchmarkMessageSender::OnStart);
    }

   private:
    void OnStart() {
      Dispatcher::Dispatch(std::make_shared<BenchmarkMessage>(this, receiver_));
    }

   private:
    Object* receiver_;
  };

  class BenchmarkMessageReceiver : public Object {
   public:
    BenchmarkMessageReceiver() : counter_{} {}

    uint64_t Counter() const noexcept {
      return counter_;
    }

   private:
    bool OnBenchmarkMessage(BenchmarkMessage&) override {
      ++counter_;
      return true;
    }

   private:
    uint64_t counter_;
  };

  const auto receiver = std::make_shared<BenchmarkMessageReceiver>();
  const auto sender = std::make_shared<BenchmarkMessageSender>(receiver.get());

  auto future = std::async(std::launch::async, [] {
    Thread::Sleep(3s);
    Dispatcher::Quit();
  });

  for (auto _ : state) {
    Dispatcher::Instance().Exec();
  }

  future.get();
}

BENCHMARK(BenchmarkDispatchingMessages);
BENCHMARK_MAIN();