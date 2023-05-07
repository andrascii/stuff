#pragma once

namespace message_driven_objects {

struct NotNullContractViolation : std::runtime_error {};

namespace details {

template <typename T, typename = void>
struct IsComparableToNullptr : std::false_type {};

template <typename T>
struct IsComparableToNullptr<
  T,
  std::enable_if_t<std::is_convertible<decltype(std::declval<T>() != nullptr), bool>::value>> : std::true_type {};

#if defined(NN_THROW_ON_CONTRACT)
inline void Expects(bool condition) {
  if (condition) {
    return;
  }

  throw NotNullContractViolation{"Contract violation"};
}
#else
inline void Expects(bool condition) {
  if (condition) {
    return;
  }

  std::terminate();
}
#endif

}// namespace details

template <typename T>
class NotNull {
 public:
  using ReturnType = std::conditional_t<
    std::is_copy_constructible<T>::value,
    T,
    const T&>;

  static_assert(
    details::IsComparableToNullptr<T>::value,
    "T cannot be compared to nullptr.");

  NotNull(std::nullptr_t) = delete;

  constexpr NotNull(T t)
      : pointer_{std::move(t)} {
    details::Expects(pointer_ != nullptr);
  }

  template <typename U>
  constexpr NotNull(U&& u)
      : pointer_{std::forward<U>(u)} {
    static_assert(std::is_convertible<U, T>::value, "U must be convertible to T");
    details::Expects(pointer_ != nullptr);
  }

  template <typename U>
  constexpr NotNull(const NotNull<U>& other) : NotNull(other.Get()) {
    static_assert(std::is_convertible<U, T>::value, "U must be convertible to T");
  }

  [[nodiscard]] constexpr ReturnType Get() const {
    details::Expects(pointer_ != nullptr);
    return pointer_;
  }

  constexpr operator T() const {
    return Get();
  }

  constexpr decltype(auto) operator->() const {
    return Get();
  }

  constexpr decltype(auto) operator*() const {
    return *Get();
  }

  NotNull& operator=(std::nullptr_t) = delete;

  // pointers only point to single objects!
  NotNull& operator++() = delete;
  NotNull& operator--() = delete;
  NotNull operator++(int) = delete;
  NotNull operator--(int) = delete;
  NotNull& operator+=(std::ptrdiff_t) = delete;
  NotNull& operator-=(std::ptrdiff_t) = delete;
  void operator[](std::ptrdiff_t) const = delete;

 private:
  T pointer_;
};

template <>
class NotNull<std::nullptr_t>;

template <typename T>
auto MakeNotNull(T&& t) noexcept {
  return NotNull<std::decay_t<T>>{std::forward<T>(t)};
}

template <class T, class U>
auto operator==(
  const NotNull<T>& lhs,
  const NotNull<U>& rhs) noexcept(noexcept(lhs.get() == rhs.get())) -> decltype(lhs.get() == rhs.get()) {
  return lhs.get() == rhs.get();
}

template <class T, class U>
auto operator!=(
  const NotNull<T>& lhs,
  const NotNull<U>& rhs) noexcept(noexcept(lhs.get() != rhs.get())) -> decltype(lhs.get() != rhs.get()) {
  return lhs.get() != rhs.get();
}

template <class T, class U>
auto operator<(
  const NotNull<T>& lhs,
  const NotNull<U>& rhs) noexcept(noexcept(lhs.get() < rhs.get())) -> decltype(lhs.get() < rhs.get()) {
  return lhs.get() < rhs.get();
}

template <class T, class U>
auto operator<=(
  const NotNull<T>& lhs,
  const NotNull<U>& rhs) noexcept(noexcept(lhs.get() <= rhs.get())) -> decltype(lhs.get() <= rhs.get()) {
  return lhs.get() <= rhs.get();
}

template <class T, class U>
auto operator>(
  const NotNull<T>& lhs,
  const NotNull<U>& rhs) noexcept(noexcept(lhs.get() > rhs.get())) -> decltype(lhs.get() > rhs.get()) {
  return lhs.get() > rhs.get();
}

template <class T, class U>
auto operator>=(
  const NotNull<T>& lhs,
  const NotNull<U>& rhs) noexcept(noexcept(lhs.get() >= rhs.get())) -> decltype(lhs.get() >= rhs.get()) {
  return lhs.get() >= rhs.get();
}

// more unwanted operators
template <class T, class U>
std::ptrdiff_t operator-(const NotNull<T>&, const NotNull<U>&) = delete;

template <class T>
NotNull<T> operator-(const NotNull<T>&, std::ptrdiff_t) = delete;

template <class T>
NotNull<T> operator+(const NotNull<T>&, std::ptrdiff_t) = delete;

template <class T>
NotNull<T> operator+(std::ptrdiff_t, const NotNull<T>&) = delete;

}// namespace common
