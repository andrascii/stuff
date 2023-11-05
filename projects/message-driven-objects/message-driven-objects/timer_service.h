#pragma once

#include "not_null.h"

namespace mdo {

using namespace std::chrono;

class Object;

class TimerService {
 public:
  class Impl;

  static TimerService* Instance();

  ~TimerService();

  int AddTimer(NotNull<Object*> object, const milliseconds& ms, bool single_shot = false);

  void RemoveTimer(int id);
  void ResetTimer(int id);

 private:
  TimerService();

 private:
  std::unique_ptr<Impl> impl_;
};

}// namespace mdo