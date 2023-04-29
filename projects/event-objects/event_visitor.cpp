#include "event_visitor.h"
#include "object.h"

namespace eo {

EventVisitor::EventVisitor(Object* object) : object_{object} {}

void EventVisitor::Visit(const Message1Event& event) {
  object_->OnMessage1Event(event);
}

void EventVisitor::Visit(const Message2Event& event) {
  object_->OnMessage2Event(event);
}

}