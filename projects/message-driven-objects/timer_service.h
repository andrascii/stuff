#pragma once

#include "thread.h"
#include "not_null.h"

namespace mdo {

class TimerService {
 public:
  TimerService();
  ~TimerService();

  void Start();
  void Stop();

  int AddTimer(NotNull<Object*> object, const std::chrono::milliseconds& ms);
  void RemoveTimer(int id);

 private:
  static int NextTimerId() noexcept;

  void Run();

 private:
  int kq_;
  mutable std::mutex mutex_;
  std::shared_ptr<Thread> managing_thread_;
  uint64_t events_count_;
};

}