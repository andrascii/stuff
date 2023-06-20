#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <list>
#include <utility>
#include <functional>

using namespace std::chrono;

using Clock = std::chrono::system_clock;
using Milliseconds = std::chrono::milliseconds;

std::ostream& Log() {
  return std::cout << std::this_thread::get_id() << ": ";
}

class Timeout {
 public:
  explicit Timeout(Milliseconds timeout, std::function<void()> callback)
      : creation_time_{duration_cast<Milliseconds>(Clock::now().time_since_epoch())},
        timeout_{timeout},
        callback_{std::move(callback)} {}

  [[nodiscard]] std::chrono::time_point<Clock, Milliseconds> CreationTime() const noexcept {
    return creation_time_;
  }

  [[nodiscard]] Milliseconds Value() const noexcept {
    return timeout_;
  }

  [[nodiscard]] const std::function<void()>& Callback() const noexcept {
    return callback_;
  }

 private:
  std::chrono::time_point<Clock, Milliseconds> creation_time_;
  Milliseconds timeout_;
  std::function<void()> callback_;
};

class TimerService {
 public:
  TimerService() {
    is_running_.clear();
  }

  ~TimerService() {
    Stop();
  }

  void Start() {
    is_running_.test_and_set();

    thread_ = std::thread([this] {
      Run();
    });

    Log() << "timer service is started\n";
  }

  void AddTimer(const Timeout& timeout) {
    std::lock_guard _{mutex_};
    timeouts_.push_back(timeout);
  }

  void Stop() {
    is_running_.clear();

    if (thread_.joinable()) {
      thread_.join();
    }

    Log() << "timer service is stopped\n";
  }

 private:
  void Run() {
    while (is_running_.test_and_set()) {
      {
        std::lock_guard _{mutex_};

        for (auto it = timeouts_.begin(); it != timeouts_.end(); ++it) {
          if (Clock::now() - it->CreationTime() >= it->Value()) {
            Log() << "executing callback for timeout\n";
            it->Callback()();
            it = timeouts_.erase(it);
          }
        }
      }

      std::this_thread::sleep_for(21ms);
    }

    Log() << "quitting timer service thread function\n";
  }

 private:
  std::atomic_flag is_running_;
  std::thread thread_;
  mutable std::mutex mutex_;
  std::list<Timeout> timeouts_;
};

void Foo(const std::chrono::time_point<Clock, Milliseconds>& task_creation_time) {
  Log()
    << "Foo was called after "
    << duration_cast<Milliseconds>(Clock::now() - task_creation_time).count()
    << " milliseconds\n";
}

void Bar(const std::chrono::time_point<Clock, Milliseconds>& task_creation_time) {
  Log()
    << "Bar was called after "
    << duration_cast<Milliseconds>(Clock::now() - task_creation_time).count()
    << " milliseconds\n";
}

int main() {
  TimerService timer_service;
  timer_service.Start();

  Log() << "adding timer for call Foo()...\n";
  timer_service.AddTimer(Timeout{
    3s,
    [now = Clock::now()]{ Foo(time_point_cast<Milliseconds>(now)); }}
  );

  Log() << "adding timer for call Bar()...\n";
  timer_service.AddTimer(Timeout{
    4s,
    [now = Clock::now()]{ Bar(time_point_cast<Milliseconds>(now)); }}
  );

  Log() << "adding timer for call Foo()...\n";
  timer_service.AddTimer(Timeout{
    5s,
    [now = Clock::now()]{ Foo(time_point_cast<Milliseconds>(now)); }}
  );

  for (uint32_t i = 0; i < 100; ++i) {
    Log() << "adding timer for call Bar()...\n";
    timer_service.AddTimer(Timeout{
      5s,
      [now = Clock::now()]{ Bar(time_point_cast<Milliseconds>(now)); }}
    );
  }

  Log() << "adding timer for call Foo()...\n";
  timer_service.AddTimer(Timeout{
    6s,
    [now = Clock::now()]{ Foo(time_point_cast<Milliseconds>(now)); }}
  );

  std::this_thread::sleep_for(10s);
}
