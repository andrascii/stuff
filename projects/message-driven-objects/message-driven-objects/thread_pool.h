#pragma once

#include "thread.h"

namespace mdo {

class ThreadPool {
public:
  ThreadPool(uint32_t thread_count);

  void Start();
  void Stop();

private:
  std::vector<std::shared_ptr<Thread>> threads_;
};

}