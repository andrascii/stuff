#pragma once

#include "iexecution_policy.h"
#include "thread_pool.h"

namespace mdo {

class ThreadPoolExecutionPolicy : public IExecutionPolicy {
 public:
  ThreadPoolExecutionPolicy(std::shared_ptr<ThreadPool> pool);

  const std::shared_ptr<mdo::Thread>& Thread() override;

 private:
  std::shared_ptr<ThreadPool> pool_;
};

}