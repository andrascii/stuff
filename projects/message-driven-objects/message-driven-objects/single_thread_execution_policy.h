#pragma once

#include "iexecution_policy.h"

namespace mdo {

class SingleThreadExecutionPolicy : public IExecutionPolicy {
 public:
  explicit SingleThreadExecutionPolicy(std::shared_ptr<mdo::Thread> thread);

  const std::shared_ptr<mdo::Thread>& Thread() override;

 private:
  std::shared_ptr<mdo::Thread> thread_;
};

}