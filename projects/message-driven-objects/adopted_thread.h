#pragma once

#include "thread.h"

namespace mdo {

class AdoptedThread : public Thread {
 public:
  explicit AdoptedThread(const ThreadDataPtr& data = nullptr);

  void Start() override;
  void Stop() override;
};

}