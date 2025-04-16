#include <iostream>

#include "circular_buffer.h"

int main() {
  CircularBuffer<uint32_t> buf{5};

  buf.Put(0);
  buf.Put(1);
  buf.Put(2);
  buf.Put(3);
  buf.Put(4);
  buf.Put(5);
  buf.Put(6);
  buf.Put(7);
  buf.Put(8);

  for (size_t i = 0; i < buf.Size(); ++i) {
    std::cout << buf[i] << " ";
  }

  std::cout << std::endl;

  while (buf.Size()) {
    std::cout << buf.Get() << " ";
  }

  std::cout << std::endl;

  return 0;
}
