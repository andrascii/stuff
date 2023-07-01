#include "timer_message.h"

#include "imessage_visitor.h"

namespace mdo {

TimerMessage::TimerMessage(int id, Object* sender, Object* receiver, PriorityType priority)
    : AbstractMessage{kTimerMessage, priority, sender, receiver},
      id_{id} {}

bool TimerMessage::Accept(IMessageVisitor& visitor) noexcept {
  return visitor.Visit(*this);
}

int TimerMessage::Id() const noexcept {
  return id_;
}

}// namespace mdo