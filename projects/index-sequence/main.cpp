#include <iostream>

/*

 1. Seq - 3
 2. Seq - 2 3
 3. Seq - 1 2 3
 4. Seq - 0 1 2 3

*/

template <typename T, T Head, T...>
struct FirstSequenceElement {
  static const T Element = Head;
};
/*
template <typename T, T... Seq>
struct IndexSequence {
  using Sequence = typename IndexSequence<T, Seq...>::Sequence;
};


template <typename T, T Head>
struct IndexSequence<T, Head> {
  using Sequence = IndexSequence<T, Head - 1, Head>;
};
*/

template <int... S>
void foo() {
  std::cout << sizeof...(S) << std::endl;
}
/*
template<typename T, T... ints>
void print_sequence(IndexSequence<T, ints...> int_seq) {
    //std::cout << "The sequence of size " << int_seq.size() << ": ";
    ((std::cout << ints << ' '), ...);
    std::cout << '\n';
}*/

template <typename Head>
void print(Head&& head) {
  std::cout << head << ' ';
}

template <typename Head, typename... T>
void print(Head&& head, T&&... args) {
  std::cout << head << ' ';
  print(std::forward<T&&>(args)...);
}

int main() {
  foo<0, 2, 3, 4, 5>();
  //print_sequence(IndexSequence<int, 3>{});

  print(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}