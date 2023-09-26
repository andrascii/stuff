#pragma once

#include <algorithm>
#include <chrono>
#include <deque>
#include <iterator>
#include <string>
#include <vector>

using namespace std::chrono;
using namespace std::literals;

namespace by_mutex {

class Sync {
 public:
  bool Push1(std::string&& v) {
    {
      std::scoped_lock _{mutex_};
      data_.push_back(std::move(v));
    }

    condition_.notify_one();
    return true;
  }

  bool Push2(std::string&& v) {
    if (mutex_.try_lock()) {
      {
        std::unique_lock _{mutex_, std::adopt_lock};
        data_.push_back(std::move(v));
      }

      condition_.notify_one();
      return true;
    }

    return false;
  }

  bool Push3(std::deque<std::string>& data) {
    if (mutex_.try_lock()) {
      {
        std::unique_lock _{mutex_, std::adopt_lock};

        if (data_.empty()) {
          swap(data, data_);
        } else {
          std::copy(
            std::make_move_iterator(data.begin()),
            std::make_move_iterator(data.end()),
            std::back_inserter(data_));

          data.clear();
        }
      }

      condition_.notify_one();
      return true;
    }

    return false;
  }

  bool Push4(std::deque<std::string>& data) {
    {
      std::scoped_lock _{mutex_};

      if (data_.empty()) {
        swap(data, data_);
      } else {
        std::copy(
          std::make_move_iterator(data.begin()),
          std::make_move_iterator(data.end()),
          std::back_inserter(data_));

        data.clear();
      }
    }

    condition_.notify_one();
    return true;
  }

  void Quit() {
    quit_ = true;
    condition_.notify_one();
  }

  bool IsQuit() const {
    return quit_;
  }

  std::deque<std::string> GetData() {
    std::unique_lock _{mutex_};
    condition_.wait(_, [this] {
      return quit_ || !data_.empty();
    });

    std::deque<std::string> data;
    swap(data, data_);
    _.unlock();

    return data;
  }

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  std::deque<std::string> data_;
  std::atomic_bool quit_{};
};

inline void Writer1(Sync& s) {
  printf("started Writer1, pushing by element and waiting on mutex\n");

  auto start = high_resolution_clock::now();
  auto prev = start;
  size_t sent = 0;
  size_t iterations = 0;

  for (;;) {
    s.Push1("Hello, World"s);
    ++sent;

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n", (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Writer2(Sync& s) {
  printf("started Writer2, pushing by element using try_lock\n");

  auto start = high_resolution_clock::now();
  auto prev = start;
  size_t sent = 0;
  size_t iterations = 0;

  for (;;) {
    s.Push2("Hello, World"s);
    ++sent;

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n", (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Writer3(Sync& s) {
  printf("started Writer3, accumulating elements while try_lock returns false, pushing accumulated data when try_lock returned true\n");

  const auto start = high_resolution_clock::now();
  auto prev = start;
  size_t sent = 0;
  size_t iterations = 0;

  std::deque<std::string> accumulated;

  for (;;) {
    accumulated.push_back("Hello, World"s);
    const auto size = accumulated.size();

    if (s.Push3(accumulated)) {
      sent += size;
    }

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n", (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Writer4(Sync& s) {
  printf("started Writer4, accumulating batch of elements of size 100 while try_lock returns false, pushing accumulated data when try_lock returned true\n");

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

    if (s.Push3(accumulated)) {
      sent += size;
    }

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n", (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Writer5(Sync& s) {
  printf("started Writer5, pushing data by batch of size 100 and waiting on mutex\n");

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

    if (s.Push4(accumulated)) {
      sent += size;
    }

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("sent per ms: %lld\n", (sent / duration_cast<milliseconds>(delta).count()));
      if (++iterations == 5) {
        s.Quit();
        printf("writer done\n");
        return;
      }
    }
  }
}

inline void Reader(Sync& s) {
  auto start = high_resolution_clock::now();
  auto prev = start;
  size_t received = 0;

  for (;;) {
    auto d = s.GetData();
    received += d.size();

    if (s.IsQuit()) {
      printf("reader done\n");
      return;
    }

    const auto current = high_resolution_clock::now();
    const auto delta = current - start;

    if (current - prev > 3s) {
      prev = current;
      printf("received per ms: %lld\n", (received / duration_cast<milliseconds>(delta).count()));
    }
  }
}

}// namespace by_mutex