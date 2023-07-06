#pragma once

#include "abstract_message.h"

namespace mdo {

class BenchmarkMessage : public AbstractMessage {
 public:
  BenchmarkMessage(Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;
};

}