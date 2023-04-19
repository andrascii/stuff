#include "abstract_object.h"
#include "message1_event.h"
#include "message2_event.h"
#include "event_visitor.h"
#include "object_registry.h"

AbstractObject::AbstractObject() {
  ObjectsRegistry::Instance().RegisterObject(this);
}

AbstractObject::~AbstractObject() {
  ObjectsRegistry::Instance().UnregisterObject(this);
}

void AbstractObject::ConnectEventReceiver(AbstractObject* object, IEvent::EventType type) {
  auto [begin, end] = objects_.equal_range(type);

  for (; begin != end; ++begin) {
    if (begin->second == object) {
      std::cout << "object already connected to this type of events\n";
      return;
    }
  }

  objects_.emplace(type, object);
}

void AbstractObject::PostEvent(const std::shared_ptr<IEvent>& event) {
  loop_.Push(event);
}

void AbstractObject::BroadcastEvent(const std::shared_ptr<IEvent>& event) {
  auto [begin, end] = objects_.equal_range(event->Type());

  for (; begin != end; ++begin) {
    const auto& [_, object] = *begin;
    object->PostEvent(event);
  }
}

void AbstractObject::Exec() {
  for (;;) {
    const auto event = loop_.Pop();
    EventVisitor visitor{ this };
    event->Accept(visitor);
  }
}

bool AbstractObject::OnMessage1Event(const Message1Event& event) {
  // do nothing here
  return true;
}

bool AbstractObject::OnMessage2Event(const Message2Event& event) {
  // do nothing here
  return true;
}