#include "text_message.h"

#include "imessage_visitor.h"

namespace mdo {

TextMessage::TextMessage(const std::string& message, Object* sender, Object* receiver)
    : AbstractMessage{kTextMessage, sender, receiver},
      message_{message} {}

bool TextMessage::Accept(IMessageVisitor& visitor) noexcept {
  return visitor.Visit(*this);
}

const std::string& TextMessage::Message() const noexcept {
  return message_;
}

}