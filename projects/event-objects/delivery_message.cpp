#include "delivery_message.h"
#include "imessage_visitor.h"

namespace eo {

bool DeliveryMessage::Accept(IMessageVisitor& visitor) const noexcept {
  return visitor.Visit(*this);
}

DeliveryMessage::DeliveryMessage(Object* sender, Object* receiver)
    : AbstractMessage{kDeliveryMessage, sender, receiver} {}

}