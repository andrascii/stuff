#include "object.h"

#include "kafka_message_notification.h"
#include "message_visitor.h"
#include "thread.h"

namespace eo {

Object::Object(Object* parent)
    : Object{Thread::Current(), parent} {
  printf("-------------- created %p --------------\n", reinterpret_cast<void*>(this));
}

Object::~Object() {
  for (const auto* child : children_) {
    delete child;
  }

  printf("-------------- destroyed %p --------------\n", reinterpret_cast<void*>(this));
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

eo::Thread* Object::Thread() const noexcept {
  return thread_.load(std::memory_order_relaxed);
}

void Object::MoveToThread(eo::Thread* thread) noexcept {
  //
  // possibly must be atomic pointer in order
  // to support ability for safe changing thread where this object lives
  //
  thread_.store(thread, std::memory_order_relaxed);
}

Object::Object(eo::Thread* thread, Object* parent)
    : parent_{nullptr},
      thread_{thread} {
  SetParent(parent);
}

bool Object::OnKafkaMessageNotification(const KafkaMessageNotification& message) {
  // do nothing here
  return false;
}

bool Object::OnDeliveryMessage(const DeliveryMessage& message) {
  // do nothing here
  return false;
}

void Object::AddChild(Object* child) noexcept {
  children_.insert(child);
}

}
