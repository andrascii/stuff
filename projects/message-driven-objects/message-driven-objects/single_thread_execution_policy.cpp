#include "single_thread_execution_policy.h"

namespace mdo {

const std::shared_ptr<mdo::Thread>& SingleThreadExecutionPolicy::Thread() {
  return thread_;
}

SingleThreadExecutionPolicy::SingleThreadExecutionPolicy(std::shared_ptr<mdo::Thread> thread)
    : thread_{std::move(thread)} {}

}