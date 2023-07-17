#pragma once

#include "thread.h"

namespace mdo {

class IExecutionPolicy {
 public:
  virtual ~IExecutionPolicy() = default;

  virtual const std::shared_ptr<mdo::Thread>& Thread() = 0;
};

}