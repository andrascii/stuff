#pragma once

#include "abstract_message.h"

namespace eo {

class DeliveryMessage : public AbstractMessage {
 public:
  DeliveryMessage(Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) const noexcept override;
};

}