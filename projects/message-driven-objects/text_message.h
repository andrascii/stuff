#pragma once

#include "abstract_message.h"

namespace message_driven_objects {

class TextMessage : public AbstractMessage {
 public:
  TextMessage(const std::string& message, Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;
  const std::string& Message() const noexcept;

 private:
  std::string message_;
};

}