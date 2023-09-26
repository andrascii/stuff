#include <chrono>
#include <iostream>
#include <thread>

#include "sync_by_flag.h"
#include "sync_by_mutex.h"

using namespace std::literals;

#if !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>

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
#endif

//
// true - reader can read, writer can't write
// false - reader can't read, writer can write
//
/*std::atomic_flag flag = ATOMIC_FLAG_INIT;
std::array<size_t, 3> queue;

void Writer() {
  std::cout << "pushing data to queue and setting flag...\n";

  for (size_t i = 0;; ++i) {
    while(flag.test(std::memory_order_relaxed));

    if (i % 2) {
      for (size_t j = 0; j < 3; ++j) {
        queue[j] = j + 1;
      }
    } else {
      std::memset(queue.data(), 0, sizeof(size_t) * 3);
    }

    std::cout << "data is ready, notifying the reader...\n";
    //flag.test_and_set(std::memory_order_release);
    flag.test_and_set(std::memory_order_relaxed);
  }
}

void Reader() {
  bool should_be_null = true;

  for (;;) {
    std::cout << "waiting for flag to be set...\n";
    //while(!flag.test(std::memory_order_acquire));
    while(!flag.test(std::memory_order_relaxed));

    std::cout << "flag was set, queue contains elements: ";

    const auto is_null = std::find_if_not(queue.begin(), queue.end(), [](size_t v) {
      return v == 0;
    }) == queue.end();

    const auto is_grow = queue[0] == 1 && queue[1] == 2 && queue[2] == 3;

    if (should_be_null && !is_null) {
      std::cout << "should be null but not null\n";
      abort();
    }

    if (!should_be_null && is_null) {
      std::cout << "should not be null but null, is_grow " << is_grow << "\n";
      abort();
    }

    should_be_null = !should_be_null;

    for (size_t elem : queue) {
      std::cout << elem << " ";
    }

    std::cout << "\nnotifying writer about data was received\n";
    flag.clear(std::memory_order_relaxed);
    //flag.clear(std::memory_order_relaxed);
  }
}*/

void ByFlag() {
  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_flag::Sync s;

    auto writer = std::thread([&s] {
      Writer1(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }

  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_flag::Sync s;

    auto writer = std::thread([&s] {
      Writer2(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }

  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_flag::Sync s;

    auto writer = std::thread([&s] {
      Writer3(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }

  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_flag::Sync s;

    auto writer = std::thread([&s] {
      Writer4(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }
}

void ByMutex() {
  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_mutex::Sync s;

    auto writer = std::thread([&s] {
      Writer1(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }

  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_mutex::Sync s;

    auto writer = std::thread([&s] {
      Writer2(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }

  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_mutex::Sync s;

    auto writer = std::thread([&s] {
      Writer3(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }

  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_mutex::Sync s;

    auto writer = std::thread([&s] {
      Writer4(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }

  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    by_mutex::Sync s;

    auto writer = std::thread([&s] {
      Writer5(s);
    });

    auto reader = std::thread([&s] {
      Reader(s);
    });

    writer.join();
    reader.join();

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }
}

int main() {
  using namespace std::chrono;

  ByFlag();
  ByMutex();
}
