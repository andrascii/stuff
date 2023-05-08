#include "loop_started.h"
#include "message_visitor.h"

namespace mdo {

LoopStarted::LoopStarted(Object* sender, Object* receiver)
    : AbstractMessage{kLoopStarted, sender, receiver} {}

bool LoopStarted::Accept(IMessageVisitor& visitor) noexcept {
  return visitor.Visit(*this);
}

}