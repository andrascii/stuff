#include "object.h"
#include "message_visitor.h"
#include "objects_registry.h"
#include "atomic_helpers.h"
#include "thread.h"

namespace message_driven_objects {

Object::Object(Object* parent)
    : Object{Thread::Current(), parent} {
  //printf("-------------- created %p --------------\n", reinterpret_cast<void*>(this));
}

Object::~Object() {
  for (const auto* child : children_) {
    delete child;
  }

  ObjectsRegistry::Instance().UnregisterObject(this);

  //printf("-------------- destroyed %p --------------\n", reinterpret_cast<void*>(this));
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

message_driven_objects::Thread* Object::Thread() const noexcept {
  return LoadRelaxed(thread_);
}

void Object::MoveToThread(message_driven_objects::Thread* thread) noexcept {
  //
  // TODO: here we must wait until all messages will be received to us from queue in previous thread
  //
  StoreRelaxed(thread_, thread);
}

Object::Object(message_driven_objects::Thread* thread, Object* parent)
    : parent_{nullptr},
      thread_{thread} {
  SetParent(parent);

  ObjectsRegistry::Instance().RegisterObject(this);
}

void Object::AddChild(Object* child) noexcept {
  children_.insert(child);
}

bool Object::OnTextMessage(const TextMessage& message) {
  // do nothing here
  return false;
}

bool Object::OnLoopStarted(const LoopStarted& message) {
  // do nothing here
  return false;
}

}
