#pragma once

#include "running_median.h"

namespace benchmarks {

class Measure {
 public:
  using TimePoint = std::chrono::time_point<
    std::chrono::high_resolution_clock,
    std::chrono::nanoseconds
  >;

  struct Metrics {
    uint64_t call_count;
    uint64_t avg_call_count;
    std::chrono::nanoseconds time_avg;
    std::chrono::nanoseconds time_min;
    std::chrono::nanoseconds time_max;
    std::chrono::nanoseconds time_median;
  };

  Measure();

  Metrics GetMetrics() const noexcept;

  void IncrementCalls() noexcept;
  uint64_t CallCount() const noexcept;

  void Reset();

 private:
  using Rep = std::chrono::nanoseconds::rep;

  uint64_t call_count_;
  uint64_t avg_call_count_per_second_;
  std::chrono::nanoseconds time_avg_ns_;
  std::chrono::nanoseconds time_min_ns_;
  std::chrono::nanoseconds time_max_ns_;
  RunningMedian<std::chrono::nanoseconds> median_;
  TimePoint first_call_time_point_;
  TimePoint previous_call_time_point_;
};

}