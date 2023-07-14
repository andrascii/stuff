#pragma once

namespace benchmarks {

class Finally final {
 public:
  using Callable = std::function<void() noexcept>;

  explicit Finally(Callable callable) : callable_{callable} {}

  Finally(const Finally& other) = delete;
  Finally& operator=(const Finally& other) = delete;

  ~Finally() {
    if(callable_) {
      callable_();
    }
  }

 private:
  Callable callable_{};
};

}