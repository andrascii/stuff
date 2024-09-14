#pragma once

#include <algorithm>
#include <chrono>
#include <deque>
#include <iterator>
#include <string>
#include <vector>

using namespace std::chrono;
using namespace std::literals;

namespace by_flag {

class Sync {
 public:
  Sync() : flag_ ATOMIC_FLAG_INIT,
           quit_{false} {}

  bool Push1(std::string&& v) {
    // if flag_ is false - we can write data
    if (!flag_.test(std::memory_order_acquire)) {
      data_.push_back(std::move(v));

      // set flag_ to true to notify reader about data is ready
      flag_.test_and_set(std::memory_order_release);
      return true;
    }

    return false;
  }

  bool Push2(std::deque<std::string>& v) {
    if (!flag_.test(std::memory_order_acquire)) {
      if (data_.empty()) {
        swap(v, data_);
      } else {
        std::copy(std::make_move_iterator(v.begin()),
                  std::make_move_iterator(v.end()),
                  std::back_inserter(data_));

        v.clear();
      }

      flag_.test_and_set(std::memory_order_release);
      return true;
    }

    return false;
  }

  void Quit() { quit_.store(true, std::memory_order_relaxed); }

  bool IsQuit() const { return quit_.load(std::memory_order_relaxed); }

  std::deque<std::string> GetData() {
    // awaiting while flag_ is false (it means that writer writes something)
    while (!quit_.load(std::memory_order_relaxed) &&
           !flag_.test(std::memory_order_acquire));

    if (quit_.load(std::memory_order_relaxed)) {
      return {};
    }

    // now when flag_ is true we can read data
    std::deque<std::string> data;
    swap(data, data_);

    // set flag_ back to false to give ability to write data
    flag_.clear(std::memory_order_release);
    return data;
  }

 private:
  std::atomic_flag flag_;
  std::deque<std::string> data_;
  std::atomic_bool quit_{};
};

inline void Writer1(Sync& s) {
  printf("started Writer1, pushing data by element if flag is false\n");

  const auto start = high_resolution_clock::now();
  auto prev = start;
  size_t sent = 0;
  size_t iterations = 0;

  std::string data = "Hello, World"s;

  for (;;) {
    if (s.Push1(std::move(data))) {
      ++sent;
      data = "Hello, World"s;
    }

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n",
             (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Writer2(Sync& s) {
  printf(
    "started Writer2, pushing accumulated data if flag is false, if its "
    "value is true - accumulating\n");

  const auto start = high_resolution_clock::now();
  auto prev = start;
  size_t sent = 0;
  size_t iterations = 0;

  std::deque<std::string> accumulated;

  for (;;) {
    accumulated.push_back("Hello, World"s);
    const auto size = accumulated.size();

    if (s.Push2(accumulated)) {
      sent += size;
    }

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n",
             (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Writer3(Sync& s) {
  printf(
    "started Writer3, pushing data by batch of size 100 with loop "
    "waiting on flag_\n");

  const auto start = high_resolution_clock::now();
  auto prev = start;
  size_t sent = 0;
  size_t iterations = 0;

  for (;;) {
    std::deque<std::string> accumulated;

    for (size_t i = 0; i < 100; ++i) {
      accumulated.push_back("Hello, World"s);
    }

    const auto size = accumulated.size();

    while (!s.Push2(accumulated)) {
    }
    sent += size;

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n",
             (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Writer4(Sync& s) {
  printf(
    "started Writer4, pushing data by batch of size 100 if cant write "
    "then accumulating (lock-free and wait-free)\n");

  const auto start = high_resolution_clock::now();
  auto prev = start;
  size_t sent = 0;
  size_t iterations = 0;

  std::deque<std::string> accumulated;

  for (;;) {
    for (size_t i = 0; i < 100; ++i) {
      accumulated.push_back("Hello, World"s);
    }

    const auto size = accumulated.size();

    if (s.Push2(accumulated)) {
      sent += size;
    }

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n",
             (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Reader(Sync& s) {
  const auto start = high_resolution_clock::now();
  auto prev = start;
  size_t received = 0;

  for (; !s.IsQuit();) {
    auto d = s.GetData();
    received += d.size();

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("received per ms: %lld\n",
             (received / duration_cast<milliseconds>(delta).count()));
    }
  }

  printf("reader done\n");
}

}// namespace by_flag