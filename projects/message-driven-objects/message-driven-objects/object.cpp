#include "object.h"

#include "invoke_slot_message.h"
#include "message_visitor.h"
#include "objects_registry.h"
#include "thread.h"
#include "timer_service.h"
#include "single_thread_execution_policy.h"

namespace mdo {

Object::Object()
    : Object{std::make_shared<SingleThreadExecutionPolicy>(Thread::Current())} {}

Object::Object(std::shared_ptr<IExecutionPolicy> execution_policy)
    : execution_policy_{std::move(execution_policy)} {
  if (!execution_policy_->Thread()) {
    execution_policy_ = std::make_shared<SingleThreadExecutionPolicy>(Thread::Current());
  }

  ObjectsRegistry::Instance().RegisterObject(this);
}

Object::~Object() {
  std::scoped_lock _{mutex_};

  for (auto it{timers_.begin()}; it != timers_.end(); ++it) {
    TimerService::Instance()->RemoveTimer(*it);
  }

  timers_.clear();

  ObjectsRegistry::Instance().UnregisterObject(this);
}

int Object::StartTimer(const std::chrono::milliseconds& ms) noexcept {
  const auto id = TimerService::Instance()->AddTimer(const_cast<Object*>(this), ms);

  {
    std::scoped_lock _{mutex_};
    timers_.insert(id);
  }

  return id;
}

void Object::KillTimer(int id) noexcept {
  {
    std::scoped_lock _{mutex_};
    timers_.erase(id);
  }

  TimerService::Instance()->RemoveTimer(id);
}

bool Object::OnMessage(const std::shared_ptr<IMessage>& message) {
  MessageVisitor visitor{this};
  return message->Accept(visitor);
}

const std::shared_ptr<Thread>& Object::Thread() const noexcept {
  std::scoped_lock _{mutex_};
  return execution_policy_->Thread();
}

void Object::SetExecutionPolicy(std::shared_ptr<IExecutionPolicy> execution_policy) {
  std::scoped_lock _{mutex_};
  execution_policy_ = std::move(execution_policy);
}

bool Object::OnInvokeSlotMessage(InvokeSlotMessage& message) {
  message.Invoke();
  return true;
}

bool Object::OnBenchmarkMessage(BenchmarkMessage&) {
  return true;
}

bool Object::OnTimerMessage(TimerMessage&) {
  return false;
}

bool Object::OnTestMessage(TestMessage&) {
  return false;
}

}// namespace mdo
