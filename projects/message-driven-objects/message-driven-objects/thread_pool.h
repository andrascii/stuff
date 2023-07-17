#pragma once

#include "thread.h"

namespace mdo {

class ThreadPool {
public:
  ThreadPool(uint32_t thread_count);

  void Start();
  void Stop();

  const std::shared_ptr<mdo::Thread>& Thread();

private:
  std::vector<std::shared_ptr<mdo::Thread>> threads_;
  std::atomic_uint32_t round_robin_counter_;
};

}