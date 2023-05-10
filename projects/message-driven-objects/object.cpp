#include "object.h"
#include "message_visitor.h"
#include "atomic_helpers.h"
#include "thread.h"
#include "invoke_slot_message.h"
#include "timer_service.h"

namespace mdo {

Object::Object(Object* parent)
    : Object{Thread::Current(), parent} {
}

Object::~Object() {
  if (parent_) {
    parent_->RemoveChild(this);
  }

  //
  // The copy is needed to avoid iterators invalidation after 'delete child;' invocation.
  // Because all children inherit Object class so when we delete them they effectively invoke 'parent_->RemoveChild(this);'.
  // This invocation leads to erasing an item from 'this->children_' an as a result we cannot more iterates over this container.
  //
  const auto children = children_;

  for (const auto* child : children) {
    delete child;
  }
}

int Object::StartTimer(const std::chrono::milliseconds& ms) const noexcept {
  return TimerService::Instance()->AddTimer(const_cast<Object*>(this), ms);
}

void Object::KillTimer(int id) const noexcept {
  TimerService::Instance()->RemoveTimer(id);
}

Object* Object::Parent() const noexcept {
  return parent_;
}

void Object::SetParent(Object* parent) {
  parent_ = parent;

  if (parent_) {
    parent_->AddChild(this);
    thread_ = parent_->Thread();
  }
}

const std::set<Object*>& Object::Children() const noexcept {
  return children_;
}

void Object::BroadcastMessage(const std::shared_ptr<IMessage>& message) {
  const auto find_parent = [](Object* object) -> Object* {
    while (object->Parent()) {
      object = object->Parent();
    }

    return object;
  };

  Object* parent = find_parent(this);
  parent->OnMessage(message);
}

bool Object::OnMessage(const std::shared_ptr<IMessage>& message) {
  MessageVisitor visitor{this};

  if (message->Accept(visitor)) {
    return true;
  }

  for (auto* child : children_) {
    if (child->OnMessage(message)) {
      return true;
    }
  }

  return false;
}

mdo::Thread* Object::Thread() const noexcept {
  return LoadRelaxed(thread_);
}

void Object::MoveToThread(mdo::Thread* thread) noexcept {
  //
  // TODO: here we must wait until all messages will be received to us from queue in previous thread
  //
  StoreRelaxed(thread_, thread);
}

Object::Object(mdo::Thread* thread, Object* parent)
    : parent_{nullptr},
      thread_{thread} {
  SetParent(parent);
}

void Object::AddChild(Object* child) noexcept {
  children_.insert(child);
}

void Object::RemoveChild(Object* child) noexcept {
  children_.erase(child);
}

bool Object::OnInvokeSlotMessage(InvokeSlotMessage& message) {
  message.Invoke();
  return true;
}

bool Object::OnTextMessage(TextMessage&) {
  // do nothing here
  return false;
}

bool Object::OnLoopStarted(LoopStarted&) {
  // do nothing here
  return false;
}

bool Object::OnTimerMessage(TimerMessage&) {
  return false;
}

}
