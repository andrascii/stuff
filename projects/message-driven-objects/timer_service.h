#pragma once

#include "not_null.h"

namespace mdo {

using namespace std::chrono;

class Object;

class TimerService {
 public:
  TimerService();
  ~TimerService();

  void Start();
  void Stop();

  int AddTimer(NotNull<Object*> object, const milliseconds& ms, bool single_shot = false);
  void RemoveTimer(int id);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}