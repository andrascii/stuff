#include <iostream>
#include <iterator>
#include <vector>

template <typename C>
void print(const C& container) {
  std::copy(
    container.begin(),
    container.end(),
    std::ostream_iterator<typename C::value_type>{std::cout, " "});

  std::cout << std::endl;
}

template <typename T, typename V>
auto Search(T first, T last, const V& value) noexcept -> T {
  T range_begin = first;
  T range_end = last;

  while (std::distance(range_begin, range_end) > 0) {
    const auto range_distance = std::distance(range_begin, range_end);
    const auto range_middle = range_distance / 2;
    const auto middle_it = std::next(range_begin, range_middle ? range_middle : 1);

    if (*middle_it == value) {
      return middle_it;
    }

    if (*middle_it > value) {
      range_end = middle_it;
    } else {
      range_begin = middle_it;
    }
  }

  return last;
}

int main() {
  auto v = std::vector{1, 2, 3, 4, 5, 5, 2, 23, 342, 4, 3, 23, 4, 3, 2, 7, 6, 67, 78, 767, 23};
  std::sort(v.begin(), v.end());
  print(v);

  const auto it = Search(v.begin(), v.end(), 767);

  if (it == v.end()) {
    std::cout << "element 767 not found\n";
  } else {
    std::cout << "element 767 found\n";
  }
}
