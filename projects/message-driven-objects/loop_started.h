#pragma once

#include "abstract_message.h"

namespace message_driven_objects {

class LoopStarted : public AbstractMessage {
 public:
  LoopStarted(Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;
};

}