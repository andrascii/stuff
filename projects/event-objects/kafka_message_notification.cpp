#include "kafka_message_notification.h"

#include "imessage_visitor.h"

namespace eo {

KafkaMessageNotification::KafkaMessageNotification(Object* sender, Object* receiver)
    : AbstractMessage{kKafkaMessageNotification, sender, receiver} {}

bool KafkaMessageNotification::Accept(IMessageVisitor& visitor) const noexcept {
  return visitor.Visit(*this);
}

}