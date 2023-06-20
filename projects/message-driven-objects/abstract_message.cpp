#include "abstract_message.h"

namespace mdo {

AbstractMessage::AbstractMessage(MessageType type, PriorityType priority, Object* sender, Object* receiver)
    : type_{type},
      priority_{priority},
      sender_{sender},
      receiver_{receiver} {}

IMessage::MessageType AbstractMessage::Type() const noexcept {
  return type_;
}

IMessage::PriorityType AbstractMessage::Priority() const noexcept {
  return priority_;
}

Object* AbstractMessage::Sender() const noexcept {
  return sender_;
}

Object* AbstractMessage::Receiver() const noexcept {
  return receiver_;
}

}