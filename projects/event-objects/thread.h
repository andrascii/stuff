#pragma once

#include "message_queue.h"
#include "object.h"
#include "thread_data.h"

namespace message_driven_objects {

static thread_local ThreadDataPtr current_thread_data = nullptr;
extern std::atomic<Thread*> the_main_thread;

class Thread : public Object {
 public:
  friend ThreadDataPtr GetThreadData(Thread* thread) noexcept;
  static Thread* Current();

  explicit Thread(ThreadDataPtr data = nullptr, Object* parent = nullptr);
  ~Thread() override;

  const std::string& Name() const noexcept;
  void SetName(const std::string& name);

  virtual void Start();
  virtual void Stop();

  bool IsRunning() const noexcept;

 protected:
  static void Run();
  static void SetCurrentThreadName(const std::string& name) noexcept;

 private:
  void StopImpl();

  void AddChild(Object* child) noexcept override;

 private:
  ThreadDataPtr data_;
  std::future<void> future_;
  std::string name_;
};

}