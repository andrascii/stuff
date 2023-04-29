#include "message1_event.h"
#include "ievent_visitor.h"

namespace eo {

Message1Event::Message1Event(Object* sender, Object* receiver) : sender_{sender}, receiver_{receiver} {}

IEvent::EventType Message1Event::Type() const noexcept {
  return kMessage1;
}

void Message1Event::Accept(IEventVisitor& visitor) const noexcept {
  visitor.Visit(*this);
}

Object* Message1Event::Sender() const noexcept {
  return sender_;
}

Object* Message1Event::Receiver() const noexcept {
  return receiver_;
}

}