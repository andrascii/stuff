#include "object.h"

#include "invoke_slot_message.h"
#include "message_visitor.h"
#include "objects_registry.h"
#include "thread.h"
#include "timer_service.h"
#include "message.h"
#include "overloaded.h"

namespace mdo {

Object::Object()
    : Object{Thread::Current()} {}

Object::Object(std::shared_ptr<mdo::Thread> thread)
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

bool Object::OnMessage(Message& message) {
  return std::visit(Overloaded{
    [this](InvokeSlotMessage& msg) -> bool {
      return OnInvokeSlotMessage(msg);
    },
    [this](TestMessage& msg) -> bool {
      return OnTestMessage(msg);
    },
    [this](BenchmarkMessage& msg) -> bool {
      return OnBenchmarkMessage(msg);
    },
    [this](SetThreadNameMessage& msg) -> bool {
      return OnSetThreadNameMessage(msg);
    },
    [this](TimerMessage& msg) -> bool {
      return OnTimerMessage(msg);
    },
    [](std::monostate& msg) -> bool {
      abort();
      return false;
    }
  }, message);
}

const std::shared_ptr<Thread>& Object::Thread() const noexcept {
  std::scoped_lock _{mutex_};
  return thread_;
}

void Object::SetThread(std::shared_ptr<mdo::Thread> thread) {
  std::scoped_lock _{mutex_};
  thread_ = std::move(thread);
}

bool Object::OnInvokeSlotMessage(InvokeSlotMessage& message) {
  message.Invoke();
  return true;
}

bool Object::OnSetThreadNameMessage(SetThreadNameMessage& message) {
  Thread::SetCurrentThreadName(message.Name());
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
