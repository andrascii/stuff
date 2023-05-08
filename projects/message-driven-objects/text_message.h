#pragma once

#include "abstract_message.h"

namespace mdo {

class TextMessage : public AbstractMessage {
 public:
  TextMessage(const std::string& message, Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;
  const std::string& Message() const noexcept;

 private:
  std::string message_;
};

}