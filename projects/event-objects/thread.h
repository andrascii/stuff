#pragma once

#include "message_queue.h"
#include "object.h"

namespace eo {

struct ThreadData {
  explicit ThreadData(uint64_t initial_ref_count = 1)
      : id{std::this_thread::get_id()},
        ref_count{initial_ref_count},
        thread{},
        is_adopted{} {}

  ~ThreadData() {
    SPDLOG_TRACE("thread object destroyed");
  }

  void Ref() noexcept {
    ref_count.fetch_add(1, std::memory_order_relaxed);
  }

  void Deref() noexcept {
    if (!ref_count.fetch_sub(1, std::memory_order_relaxed)) {
      delete this;
    }
  }

  MessageQueue event_queue;
  std::atomic<std::thread::id> id;
  std::atomic_uint64_t ref_count;
  std::atomic<Thread*> thread;
  bool is_adopted;
};

static thread_local ThreadData* current_thread_data = nullptr;
extern std::atomic<Thread*> the_main_thread;

class Thread : public Object {
 public:
  friend ThreadData* GetThreadData(Thread* thread) noexcept;

  static Thread* Current();

  explicit Thread(ThreadData* data = nullptr, Object* parent = nullptr);
  ~Thread();

  virtual void Start();
  virtual void Stop();

 protected:
  static void ThreadEntryPoint();

 private:
  void StopImpl();

 private:
  ThreadData* data_;
  std::future<void> future_;
};

}