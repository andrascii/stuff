#include <iostream>
#include <type_traits>

bool my_less(const auto& lhs, const auto& rhs) {
  return lhs < rhs;
}

template <typename T>
concept IsPointer = std::is_pointer_v<T>;

template <typename T>
concept IsFloatingPoint = std::is_floating_point_v<T>;

template <typename T>
concept CanCallRunMethod = requires(T p) { p.Run(); };

template <typename T> requires(!IsPointer<T>)
T MaxValue(T a, T b) {
  return a > b ? a : b;
}

template <typename T> requires IsPointer<T>
auto MaxValue(T a, T b) requires std::three_way_comparable_with<decltype(*a), decltype(*b)> {
  return MaxValue(*a, *b);
}

auto IsEqual(const auto& a, const auto& b) {
  return a == b;
}

auto IsEqual(const IsPointer auto& a, const IsPointer auto& b) {
  return IsEqual(*a, *b);
}

auto IsEqual(const IsFloatingPoint auto& a, const IsFloatingPoint auto& b) requires(std::is_same_v<decltype(a), decltype(b)>) {
  using CommonType = std::common_type_t<decltype(a), decltype(b)>;
  return a - b <= std::numeric_limits<CommonType>::epsilon();
}

template <typename T>
auto CallRun(const T& object) requires CanCallRunMethod<T> {
  return object.Run();
}

struct HaveRun {
  void Run() const {
    std::cout << "run\n";
  }
};

struct HaveExecute {
  void Execute() const {
    std::cout << "run\n";
  }
};

template <typename Coll, typename T>
concept SupportsPushBack = requires(Coll c, T v) {
  c.push_back(v);
};

template <typename Coll, typename T>
void Add(Coll& coll, const T& val) requires (SupportsPushBack<Coll, T> && std::convertible_to<T, typename Coll::value_type>) {
  coll.push_back(val);
}

struct X {
  static const int n = 1;
  static constexpr int m = 4;
};

const int* p = &X::n, * q = &X::m;

void Func(const int& v) {
  std::cout << "passed: " << v << std::endl;
}

int main() {
  (void)p;
  (void)q;

  std::cout << "p: " << p << std::endl;
  Func(X::n);

  std::vector v{ 1,12,2,7,9,0,3,4,5 };
  std::vector v2 = v;
  std::sort(v.begin(), v.end(), my_less<int, int>);
  std::ranges::sort(v2);

  for (auto e : v) {
    std::cout << e << " ";
  }

  std::cout << std::endl;

  int x = 42;
  int y = 77;
  int* xp = &x;
  int* yp = &y;

  std::cout << "MaxValue(x, y): " << MaxValue(x, y) << std::endl;
  std::cout << "MaxValue(&x, &y): " << MaxValue(&x, &y) << std::endl;
  std::cout << "MaxValue(&xp, &yp): " << MaxValue(&xp, &yp) << std::endl; // recursive calls MaxValue<T**> => MaxValue<T*> => MaxValue<T>
  std::cout << std::boolalpha << "IsEqual(1.0, 1.0): " << IsEqual(1.0, 1.0) << std::endl;
  std::cout << std::boolalpha << "IsEqual(1.1, 1.0): " << IsEqual(1.1, 1.0) << std::endl;
  std::cout << std::boolalpha << "IsEqual(x, y): " << IsEqual(x, y) << std::endl;
  std::cout << std::boolalpha << "IsEqual(xp, yp): " << IsEqual(xp, yp) << std::endl;
  std::cout << std::boolalpha << "IsEqual(&xp, &yp): " << IsEqual(&xp, &yp) << std::endl;

  CallRun(HaveRun{});
  //CallRun(HaveExecute{});

  /*std::set<int> set;
  std::vector<std::atomic<int>> aiVec;
  std::atomic<int> ai{ 42 };
  Add(set, 1);
  Add(aiVec, ai);*/

  return 0;
}