#include "loop_started.h"
#include "message_visitor.h"

namespace mdo {

LoopStartedMessage::LoopStartedMessage(Object* sender, Object* receiver)
    : AbstractMessage{kLoopStarted, sender, receiver} {}

bool LoopStartedMessage::Accept(IMessageVisitor& visitor) noexcept {
  return visitor.Visit(*this);
}

}