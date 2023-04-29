#pragma once

#include "event_loop.h"

namespace eo {

class Thread;

struct ThreadData {
  explicit ThreadData(uint64_t initial_ref_count = 1)
      : id{std::this_thread::get_id()},
        ref_count{initial_ref_count},
        thread{},
        is_adopted{} {}

  void Ref() noexcept {
    ref_count.fetch_add(1, std::memory_order_relaxed);
  }

  void Deref() noexcept {
    if (!ref_count.fetch_sub(1, std::memory_order_relaxed)) {
      delete this;
    }
  }

  EventLoop event_loop;
  std::atomic<std::thread::id> id;
  std::atomic_uint64_t ref_count;
  std::atomic<Thread*> thread;
  bool is_adopted;
};

static thread_local ThreadData* current_thread_data = nullptr;
extern std::atomic<Thread*> the_main_thread;

class Thread {
 public:
  friend ThreadData* GetThreadData(Thread* thread) noexcept;

  static Thread* Current();

  explicit Thread(ThreadData* data = nullptr);
  ~Thread();

  virtual void Start();
  virtual void Stop();

 protected:
  static void Run();

 private:
  std::atomic<ThreadData*> data_;
  std::future<void> future_;
};

}