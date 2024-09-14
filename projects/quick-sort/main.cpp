#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std::literals;

template <typename It>
void quicksort(It low, It high) {
  const auto distance = std::distance(low, high);
  It middle = std::next(low, distance);
}

int main() {
  // fill the vectors with random numbers
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dis(0, 9);

  std::vector<int> v1(10), v2(10);
  std::generate(v1.begin(), v1.end(), std::bind(dis, std::ref(mt)));
  std::generate(v2.begin(), v2.end(), std::bind(dis, std::ref(mt)));
}
