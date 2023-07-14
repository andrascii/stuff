#include "measure.h"

namespace benchmarks {

Measure::Measure(){
  Reset();
}

Measure::Metrics Measure::GetMetrics() const noexcept {
  Metrics metrics{
    call_count_,
    avg_call_count_per_second_,
    time_avg_ns_,
    time_min_ns_,
    time_max_ns_,
    median_.GetMedian()
  };

  return metrics;
}

void Measure::IncrementCalls() noexcept {
  using namespace std::chrono;
  const auto current_time_point = high_resolution_clock::now();

  if (!first_call_time_point_.time_since_epoch().count()) {
    first_call_time_point_ = current_time_point;
  }

  if (!previous_call_time_point_.time_since_epoch().count()) {
    previous_call_time_point_ = current_time_point;
    ++call_count_;
    return;
  }

  const auto one_call_measure_delta = current_time_point - previous_call_time_point_;
  const auto common_measure_delta = current_time_point - first_call_time_point_;

  ++call_count_;
  const auto delimiter = duration_cast<milliseconds>(common_measure_delta).count();
  avg_call_count_per_second_ = (call_count_ / (delimiter ? delimiter : 1)) * 1000;
  time_avg_ns_ = one_call_measure_delta / call_count_;

  if (one_call_measure_delta < time_min_ns_) {
    time_min_ns_ = one_call_measure_delta;
  }

  if (one_call_measure_delta > time_max_ns_) {
    time_max_ns_ = one_call_measure_delta;
  }

  median_.Push(one_call_measure_delta);
}

uint64_t Measure::CallCount() const noexcept {
  return call_count_;
}

void Measure::Reset() {
  call_count_ = std::numeric_limits<uint64_t>::min();
  avg_call_count_per_second_ = std::numeric_limits<uint64_t>::min();
  time_avg_ns_ = std::chrono::nanoseconds{std::numeric_limits<Rep>::min()};
  time_min_ns_ = std::chrono::nanoseconds{std::numeric_limits<Rep>::max()};
  time_max_ns_ = std::chrono::nanoseconds{std::numeric_limits<Rep>::min()};
  previous_call_time_point_ = TimePoint{};
  first_call_time_point_ = TimePoint{};
}

}