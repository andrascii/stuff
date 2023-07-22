#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <algorithm>

using namespace std::literals;

[[maybe_unused]] const auto print = [](const auto rem, const auto& v) {
  std::cout << rem;
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << '\n';
};

template <typename T1, typename T2, typename T3>
void MergeSort(T1 first1, T1 last1, T2 first2, T2 last2, T3 out) {
  for (; first1 != last1; ++out) {
    if (first2 == last2) {
      return std::copy(first1, last1, out);
    }

    if (*first1 < *first2) {
      *out = *first1;
      ++first1;
    } else {
      *out = *first2;
      ++first2;
    }
  }

  return std::copy(first2, last2, out);
}

void Foo() {
  using namespace std::chrono;

  int fd = open("file.dat", O_SYNC | O_CREAT | O_RDWR | F_NOCACHE, S_IRUSR | S_IWUSR);

  if (fd == -1) {
    std::cerr << "can't open file\n";
    return;
  }

  struct Block {
    size_t magic_start = 0;
    size_t payload = 0;
    size_t magic_end = 0;
  };

  Block block;

  const auto start = high_resolution_clock::now();

  for (size_t i = 0; i < 1000; ++i) {
    block.payload = i;
    write(fd, &block, sizeof(Block));
  }

  const auto end = high_resolution_clock::now();

  const auto delta = end - start;
  std::cout << "1000 write operations took " << delta.count() << " nanoseconds\n";
  std::cout << "1 write operation took " << delta.count() / 1000 << " nanoseconds\n";

  close(fd);
}

int main() {
  // fill the vectors with random numbers
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dis(0, 9);

  std::vector<int> v1(10), v2(10);
  std::generate(v1.begin(), v1.end(), std::bind(dis, std::ref(mt)));
  std::generate(v2.begin(), v2.end(), std::bind(dis, std::ref(mt)));

  print("Originally:\nv1: ", v1);
  print("v2: ", v2);

  std::sort(v1.begin(), v1.end());
  std::sort(v2.begin(), v2.end());

  print("\nAfter sorting:\nv1: ", v1);
  print("v2: ", v2);

  // merge
  std::vector<int> dst;
  std::merge(v1.begin(), v1.end(), v2.begin(), v2.end(), std::back_inserter(dst));

  print("\nAfter merging:\ndst: ", dst);*/
}
