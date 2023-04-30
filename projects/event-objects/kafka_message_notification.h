#pragma once

#include "abstract_message.h"

namespace eo {

class IMessageVisitor;

class KafkaMessageNotification : public AbstractMessage {
 public:
  KafkaMessageNotification(Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) const noexcept override;
};

}