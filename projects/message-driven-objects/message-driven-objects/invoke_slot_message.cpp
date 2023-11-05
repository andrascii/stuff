#include "invoke_slot_message.h"

namespace mdo {

InvokeSlotMessage::InvokeSlotMessage(const std::function<void()>& f,
                                     Object* sender,
                                     Object* receiver)
    : MessageBase{sender, receiver},
      f_{f} {}

void InvokeSlotMessage::Invoke() const { f_(); }

}// namespace mdo