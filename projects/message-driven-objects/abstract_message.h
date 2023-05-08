#pragma once

#include "imessage.h"

namespace mdo {

class AbstractMessage : public IMessage {
 public:
  AbstractMessage(MessageType type, Object* sender, Object* receiver);

  [[nodiscard]] MessageType Type() const noexcept override;
  [[nodiscard]] Object* Sender() const noexcept override;
  [[nodiscard]] Object* Receiver() const noexcept override;

 private:
  MessageType type_;
  Object* sender_;
  Object* receiver_;
};

}