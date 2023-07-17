#include "thread_pool_execution_policy.h"

namespace mdo {

ThreadPoolExecutionPolicy::ThreadPoolExecutionPolicy(std::shared_ptr<ThreadPool> pool)
    : pool_{std::move(pool)} {}

const std::shared_ptr<mdo::Thread>& ThreadPoolExecutionPolicy::Thread() {
  return pool_->Thread();
}

}