#pragma once

namespace mdo {

template <typename T, typename M = std::mutex>
class Locked {
 public:
  template <typename... Args>
  explicit Locked(Args&&... args) : data_{std::forward<Args>(args)...} {}

  template <typename F>
  class Access {
   public:
    Access(F&& data, M& mutex) : data_{data},
                                 locker_{mutex} {}

    T* operator->() noexcept { return &data_; }

    const T* operator->() const noexcept { return &data_; }

    T& operator*() noexcept { return data_; }

    const T& operator*() const noexcept { return data_; }

    operator F&() noexcept { return data_; }

    operator const F&() const noexcept { return data_; }

   private:
    F data_;
    std::scoped_lock<M> locker_;
  };

  template <typename F>
  Access(F&, M) -> Access<F&>;

  template <typename F>
  Access(const F&, M) -> Access<const F&>;

  auto operator->() noexcept { return Access{data_, mutex_}; }

  auto operator->() const noexcept { return Access{data_, mutex_}; }

  auto operator*() noexcept { return Access{data_, mutex_}; }

  auto operator*() const noexcept { return Access{data_, mutex_}; }

  template <typename F>
  Locked& operator=(F&& f) {
    std::scoped_lock _{mutex_};
    data_ = std::forward<F>(f);
    return *this;
  }

 private:
  T data_;
  mutable M mutex_;
};

}// namespace mdo