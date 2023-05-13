#pragma once

#include "abstract_message.h"

namespace mdo {

class LoopStartedMessage : public AbstractMessage {
 public:
  LoopStartedMessage(Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;
};

}