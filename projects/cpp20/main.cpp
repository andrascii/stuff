#include <iostream>
#include <type_traits>

template <typename T>
concept IsPointer = std::is_pointer_v<T>;

template <typename T>
T MaxValue(const T& a, const T& b) { return a > b ? a : b; }

template <typename T> requires IsPointer<T>
auto MaxValue(const T& a, const T& b) {
  return MaxValue(*a, *b);
}

/*

// №1
template <typename T>
auto MaxValue(const T& a, const T& b) requires IsPointer<T> {
  return MaxValue(*a, *b);
}

// №2
template <IsPointer T>
auto MaxValue(const T& a, const T& b) {
  return Max(*a, *b);
}

// №3
auto MaxValue(IsPointer auto a, IsPointer auto b) {
  return Max(*a, *b);
}

 */

int main() {
  int x = 42;
  int y = 43;
  int* xp = &x;
  int* yp = &y;

  std::cout << "MaxValue(x, y): " << MaxValue(x, y) << std::endl;
  std::cout << "MaxValue(&x, &y): " << MaxValue(&x, &y) << std::endl;
  std::cout << "MaxValue(&xp, &yp): " << MaxValue(&xp, &yp) << std::endl;
}