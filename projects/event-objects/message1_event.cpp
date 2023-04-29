#include "message1_event.h"
#include "ievent_visitor.h"

namespace eo {

IEvent::EventType Message1Event::Type() const noexcept {
  return kMessage1;
}

void Message1Event::Accept(IEventVisitor& visitor) const noexcept {
  visitor.Visit(*this);
}

}