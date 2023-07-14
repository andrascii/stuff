#pragma once

namespace benchmarks {

template <class T>
class RunningMedian final {
 public:
  RunningMedian();

  auto Push(T val) noexcept -> void;

  auto GetMedian() const noexcept -> T;

 private:
  std::priority_queue<T, std::vector<T>, std::greater<>> upper_;
  std::priority_queue<T, std::vector<T>, std::less<>> lower_;
};

template <class T>
RunningMedian<T>::RunningMedian() {
  upper_.push(std::numeric_limits<T>::max());
  lower_.push(std::numeric_limits<T>::min());
}

template <class T>
auto RunningMedian<T>::Push(T val) noexcept -> void {
  try {
    if (val >= upper_.top()) {
      upper_.push(val);
    } else {
      lower_.push(val);
    }

    if (upper_.size() - lower_.size() == 2) {
      lower_.push(upper_.top());
      upper_.pop();
    } else if (lower_.size() - upper_.size() == 2) {
      upper_.push(lower_.top());
      lower_.pop();
    }
  } catch (std::exception const& ex) {
    LOG_CRITICAL("{:s} while updating running median", ex.what());
    _exit(EXIT_FAILURE);
  }
}

// ReSharper disable once CppNotAllPathsReturnValue
template <class T>
auto RunningMedian<T>::GetMedian() const noexcept -> T {
  try {
    if (upper_.size() == lower_.size()) {
      return (upper_.top() + lower_.top()) / 2;
    }
    if (upper_.size() > lower_.size()) {
      return upper_.top();
    }
    return lower_.top();
  } catch (std::exception const& ex) {
    LOG_CRITICAL("{:s} while calculating running median", ex.what());
    _exit(EXIT_FAILURE);
  }
}

}