#include "invoke_slot_message.h"
#include "imessage_visitor.h"

namespace message_driven_objects {

InvokeSlotMessage::InvokeSlotMessage(const std::function<void()>& f, Object* sender, Object* receiver)
    : AbstractMessage{kInvokeSlotMessage, sender, receiver},
      f_{f} {}

bool InvokeSlotMessage::Accept(IMessageVisitor& visitor) noexcept {
  return visitor.Visit(*this);
}

void InvokeSlotMessage::Invoke() const {
  f_();
}

}