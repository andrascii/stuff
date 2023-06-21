#include "object.h"

#include "invoke_slot_message.h"
#include "message_visitor.h"
#include "objects_registry.h"
#include "thread.h"
#include "timer_service.h"

namespace mdo {

Object::Object()
    : Object{Thread::Current()} {}

Object::Object(mdo::Thread* thread)
    : thread_{thread} {
  ObjectsRegistry::Instance().RegisterObject(this);
}

Object::~Object() {
  auto timers_access = *timers_;

  for (auto it{timers_access->begin()}; it != timers_access->end(); ++it) {
    TimerService::Instance()->RemoveTimer(*it);
  }

  timers_access->clear();

  ObjectsRegistry::Instance().UnregisterObject(this);
}

int Object::StartTimer(const std::chrono::milliseconds& ms) noexcept {
  const auto id = TimerService::Instance()->AddTimer(const_cast<Object*>(this), ms);
  timers_->insert(id);
  return id;
}

void Object::KillTimer(int id) noexcept {
  timers_->erase(id);
  TimerService::Instance()->RemoveTimer(id);
}

bool Object::OnMessage(const std::shared_ptr<IMessage>& message) {
  MessageVisitor visitor{this};
  return message->Accept(visitor);
}

mdo::Thread* Object::Thread() const noexcept {
  return *thread_;
}

void Object::MoveToThread(mdo::Thread* thread) noexcept {
  //
  // TODO: here we must wait until all messages will be received to us from queue in previous thread
  //
  thread_ = thread;
}

bool Object::OnInvokeSlotMessage(InvokeSlotMessage& message) {
  message.Invoke();
  return true;
}

bool Object::OnTimerMessage(TimerMessage&) {
  return false;
}

}// namespace mdo
