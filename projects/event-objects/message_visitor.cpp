#include "message_visitor.h"

#include "object.h"

namespace eo {

MessageVisitor::MessageVisitor(Object* object)
    : object_{object} {}

bool MessageVisitor::Visit(const KafkaMessageNotification& message) {
  return object_->OnKafkaMessageNotification(message);
}

bool MessageVisitor::Visit(const DeliveryMessage& message) {
  return object_->OnDeliveryMessage(message);
}

}