#pragma once

#include "thread.h"

namespace eo {

class AdoptedThread : public Thread {
 public:
  explicit AdoptedThread(ThreadData* data = nullptr);

  void Start() override;
  void Stop() override;
};

}