#include "object.h"
#include "event_visitor.h"
#include "message1_event.h"
#include "message2_event.h"
#include "object_registry.h"

namespace eo {

Object::Object(Object* parent) : parent_{nullptr} {
  SetParent(parent);
  ObjectsRegistry::Instance().RegisterObject(this);
  thread_ = Thread::Current();
}

Object::~Object() {
  for (const auto* child : children_) {
    delete child;
  }

  ObjectsRegistry::Instance().UnregisterObject(this);
}

Object* Object::Parent() const noexcept {
  return parent_;
}

void Object::SetParent(Object* parent) {
  parent_ = parent;

  if (parent_) {
    parent_->AddChild(this);
  }
}

void Object::BroadcastEvent(const std::shared_ptr<IEvent>& event) {
  const auto find_parent = [](Object* object) -> Object* {
    while (object->Parent()) {
      object = object->Parent();
    }

    return object;
  };

  Object* parent = find_parent(this);
  parent->Event(event);
}

bool Object::Event(const std::shared_ptr<IEvent>& event) {
  EventVisitor visitor{this};
  event->Accept(visitor);

  for (Object* child : children_) {
    if (child->Event(event)) {
      return true;
    }
  }

  return false;
}

Thread* Object::ThreadAffinity() const noexcept {
  return thread_;
}

void Object::MoveToThread(Thread* thread) noexcept {
  //
  // TODO: implement
  //
  thread_ = thread;
}

bool Object::OnMessage1Event(const Message1Event& event) {
  // do nothing here
  return false;
}

bool Object::OnMessage2Event(const Message2Event& event) {
  // do nothing here
  return false;
}

void Object::AddChild(Object* child) noexcept {
  children_.insert(child);
}

}
