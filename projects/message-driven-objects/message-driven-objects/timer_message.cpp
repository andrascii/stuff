#include "timer_message.h"

namespace mdo {

TimerMessage::TimerMessage(int id, Object* sender, Object* receiver)
    : MessageBase{sender, receiver},
      id_{id} {}

int TimerMessage::Id() const noexcept {
  return id_;
}

}// namespace mdo