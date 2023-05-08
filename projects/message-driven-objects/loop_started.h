#pragma once

#include "abstract_message.h"

namespace mdo {

class LoopStarted : public AbstractMessage {
 public:
  LoopStarted(Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;
};

}