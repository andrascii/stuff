#pragma once

#include "locked.h"
#include "message_queue.h"

namespace mdo {

class Thread;

class ThreadData {
 public:
  ThreadData();

  void lock() const;
  void unlock() const;

  MessageQueue& Queue() noexcept;
  const MessageQueue& Queue() const noexcept;

  const std::thread::id& Id() const noexcept;
  void SetId(const std::thread::id& id);

  const std::shared_ptr<mdo::Thread>& Thread() const noexcept;
  void SetThread(std::shared_ptr<mdo::Thread> thread) noexcept;

  bool InterruptionRequested() const noexcept;
  void SetInterruptionRequest(bool value = true) noexcept;

  bool IsAdopted() const noexcept;
  void SetIsAdopted(bool value) noexcept;

 private:
  MessageQueue queue_;
  mutable std::recursive_mutex mutex_;
  std::thread::id id_;
  std::shared_ptr<mdo::Thread> thread_;
  bool interruption_requested_;
  bool is_adopted_;
};

}// namespace mdo