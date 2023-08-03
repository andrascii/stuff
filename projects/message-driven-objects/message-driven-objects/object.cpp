#include "object.h"

#include "invoke_slot_message.h"
#include "objects_registry.h"
#include "thread.h"
#include "timer_service.h"
#include "message.h"
#include "overloaded.h"

namespace mdo {

Object::Object()
    : Object{Thread::Current()} {}

Object::Object(mdo::Thread* thread)
    : thread_{std::move(thread)} {
  if (!Thread()) {
    thread_ = Thread::Current();
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

void Object::ResetTimer(int id) const noexcept {
  TimerService::Instance()->ResetTimer(id);
}

void Object::OnMessage(Message& message) {
  std::visit(Overloaded{
    [this](InvokeSlotMessage& msg) {
      OnInvokeSlotMessage(msg);
    },
    [this](TestMessage& msg) {
      OnTestMessage(msg);
    },
    [this](BenchmarkMessage& msg) {
      OnBenchmarkMessage(msg);
    },
    [this](TimerMessage& msg) {
      OnTimerMessage(msg);
    },
    [](SetThreadNameMessage&) {
      abort();
    },
    [](std::monostate&) {
      abort();
    }
  }, message);
}

mdo::Thread* Object::Thread() const noexcept {
  std::scoped_lock _{mutex_};
  return thread_;
}

void Object::SetThread(mdo::Thread* thread) {
  std::scoped_lock _{mutex_};
  thread_ = thread;
}

void Object::OnInvokeSlotMessage(InvokeSlotMessage& message) {
  message.Invoke();
}

void Object::OnBenchmarkMessage(BenchmarkMessage&) {}

void Object::OnTimerMessage(TimerMessage&) {}

void Object::OnTestMessage(TestMessage&) {}

}// namespace mdo
