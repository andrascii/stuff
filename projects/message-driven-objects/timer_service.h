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
  int NextTimerId() const noexcept;

  void Run();

 private:
  struct TimerContext {
    Object* object;
  };

  int kq_;
  mutable std::mutex mutex_;
  std::shared_ptr<Thread> managing_thread_;
  std::vector<struct kevent> kevents_;
  std::map<int, TimerContext> timer_contexts_;
};

}