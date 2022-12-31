#include <functional>
#include <iostream>
#include <vector>
#include <unordered_map>

template <typename T, typename V>
auto Search(T first, T last, const V& value) noexcept -> T {
  T range_begin = first;
  T range_end = last;

  while (std::distance(range_begin, range_end) > 0) {
    const auto distance = std::distance(range_begin, range_end);
    const auto step = distance / 2;
    auto it = std::next(range_begin, step ? step : 1);

    if (*it == value) {
      return it;
    }

    if (*it > value) {
      range_end = --it;
    } else {
      range_begin = ++it;
    }
  }

  return last;
}

//
// Find all pairs of numbers whose sum is equal to a given number.
//

using IndexPair = std::pair<size_t, size_t>;

std::vector<IndexPair> Find1(const std::vector<int>& v, int sum) {
  std::vector<IndexPair> result;
  std::unordered_map<size_t, std::vector<size_t>> map;

  for (size_t i = 0; i < v.size(); ++i) {
    map[v[i]].push_back(i);
  }

  for (size_t i = 0; i < v.size(); ++i) {
    const auto searching = sum - v[i];
    const auto it = map.find(searching);

    if (it == end(map)) {
      continue;
    }

    for (auto index : it->second) {
      if (i == index) {
        continue;
      }

      result.emplace_back(i, index);
    }
  }

  return result;
}

std::vector<IndexPair> Find2(std::vector<int> v, int sum) {
  std::vector<IndexPair> result;
  std::sort(v.begin(), v.end());

  for (size_t i = 0; i < v.size(); ++i) {
    const auto searching = sum - v[i];
    const auto it = Search(v.begin(), v.end(), searching);

    if (it == v.end()) {
      continue;
    }

    const auto found_element_index = std::distance(v.begin(), it);

    if (found_element_index == static_cast<long>(i)) {
      continue;
    }

    result.emplace_back(i, found_element_index);
  }

  return result;
}

void PrintPairs(const std::vector<int>& v, const std::vector<IndexPair>& pairs, int sum) {
  if (pairs.empty()) {
    std::cout << "not found pairs of numbers whose sum is equal to " << sum << ":" << std::endl;
  } else {
    std::cout << "pairs of numbers whose sum is equal to " << sum << ":" << std::endl;
    for (const auto& [i1, i2] : pairs) {
      std::cout << "[" << i1 << "; " << i2 << "] - " << v[i1] << " + " << v[i2] << " = " << v[i1] + v[i2] << std::endl;
    }
  }

  std::cout << std::endl;
}

int main() {
  const auto v = std::vector{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

  int sum = 0;

  std::cout << "type in the sum to found indexes of numbers whose sum must be equal: ";
  std::cin >> sum;

  const auto pairs1 = Find1(v, sum);
  PrintPairs(v, pairs1, sum);

  const auto pairs2 = Find2(v, sum);
  PrintPairs(v, pairs2, sum);
}
