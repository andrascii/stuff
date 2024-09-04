#pragma once

#include <stop_token>

class Runnable final {
 public:
  using ToRun = std::function<void()>;

  explicit Runnable(ToRun&& to_run)
      : to_run_{std::move(to_run)},
        stop_{} {}

  ~Runnable() {
    Stop();
  }

  void Run() {
    if (future_.valid()) {
      std::cerr << "already started\n";
      return;
    }

    stop_.store(false, std::memory_order_relaxed);

    future_ = std::async(std::launch::async, [this] {
      RunWrapper();
    });
  }

  void Stop() {
    if (!future_.valid()) {
      return;
    }

    stop_.store(true, std::memory_order_relaxed);

    future_.wait();
  }

 private:
  void RunWrapper() {
    while (!stop_.load(std::memory_order_relaxed)) {
      to_run_();
    }
  }

 private:
  ToRun to_run_;
  std::future<void> future_;
  std::atomic_bool stop_;
};