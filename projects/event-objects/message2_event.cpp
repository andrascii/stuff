#include "message2_event.h"
#include "ievent_visitor.h"

namespace eo {

Message2Event::Message2Event(Object* sender, Object* receiver) : sender_{sender}, receiver_{receiver} {}

IEvent::EventType Message2Event::Type() const noexcept {
  return kMessage2;
}

void Message2Event::Accept(IEventVisitor& visitor) const noexcept {
  visitor.Visit(*this);
}

Object* Message2Event::Sender() const noexcept {
  return sender_;
}

Object* Message2Event::Receiver() const noexcept {
  return receiver_;
}

}