#include "message2_event.h"
#include "ievent_visitor.h"

namespace eo {

IEvent::EventType Message2Event::Type() const noexcept {
  return kMessage2;
}

void Message2Event::Accept(IEventVisitor& visitor) const noexcept {
  visitor.Visit(*this);
}

}