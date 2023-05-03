#include "text_message.h"

#include "imessage_visitor.h"

namespace message_driven_objects {

TextMessage::TextMessage(const std::string& message, Object* sender, Object* receiver)
    : AbstractMessage{kTextMessage, sender, receiver},
      message_{message} {}

bool TextMessage::Accept(IMessageVisitor& visitor) const noexcept {
  return visitor.Visit(*this);
}

const std::string& TextMessage::Message() const noexcept {
  return message_;
}

}