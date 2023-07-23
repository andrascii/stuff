#pragma once

#include "thread.h"

namespace mdo {

class AdoptedThread : public Thread {
 public:
  explicit AdoptedThread(const std::shared_ptr<ThreadData>& data = nullptr);

  void Start() override;
};

}// namespace mdo