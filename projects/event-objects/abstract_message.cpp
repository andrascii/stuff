#include "abstract_message.h"

namespace message_driven_objects {

AbstractMessage::AbstractMessage(MessageType type, Object* sender, Object* receiver)
    : type_{type},
      sender_{sender},
      receiver_{receiver} {}

IMessage::MessageType AbstractMessage::Type() const noexcept {
  return type_;
}

Object* AbstractMessage::Sender() const noexcept {
  return sender_;
}

Object* AbstractMessage::Receiver() const noexcept {
  return receiver_;
}

}