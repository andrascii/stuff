#include "thread_data.h"

#include "atomic_helpers.h"
#include "thread.h"

namespace mdo {

ThreadData::ThreadData()
    : id_{std::this_thread::get_id()},
      thread_{},
      interruption_requested_{false},
      is_adopted_{false} {}

ThreadData::~ThreadData() {
  std::scoped_lock _{*this};

  if (is_adopted_) {
    delete thread_;
  }
}

void ThreadData::lock() const {
  mutex_.lock();
}

void ThreadData::unlock() const {
  mutex_.unlock();
}

MessageQueue& ThreadData::Queue() noexcept {
  return queue_;
}

const MessageQueue& ThreadData::Queue() const noexcept {
  return queue_;
}

const std::thread::id& ThreadData::Id() const noexcept {
  std::scoped_lock _{*this};
  return id_;
}

void ThreadData::SetId(const std::thread::id& id) {
  std::scoped_lock _{*this};
  id_ = id;
}

mdo::Thread* ThreadData::Thread() const noexcept {
  std::scoped_lock _{*this};
  return thread_;
}

void ThreadData::SetThread(mdo::Thread* thread) noexcept {
  std::scoped_lock _{*this};
  thread_ = thread;
}

bool ThreadData::InterruptionRequested() const noexcept {
  std::scoped_lock _{*this};
  return interruption_requested_;
}

void ThreadData::SetInterruptionRequest(bool value) noexcept {
  std::scoped_lock _{*this};
  interruption_requested_ = value;
}

bool ThreadData::IsAdopted() const noexcept {
  std::scoped_lock _{*this};
  return is_adopted_;
}

void ThreadData::SetIsAdopted(bool value) noexcept {
  std::scoped_lock _{*this};
  is_adopted_ = value;
}

}// namespace mdo