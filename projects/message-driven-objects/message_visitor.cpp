#include "message_visitor.h"
#include "object.h"

namespace mdo {

MessageVisitor::MessageVisitor(Object* object)
    : object_{object} {}

bool MessageVisitor::Visit(TextMessage& message) {
  return object_->OnTextMessage(message);
}

bool MessageVisitor::Visit(LoopStartedMessage& message) {
  return object_->OnLoopStartedMessage(message);
}

bool MessageVisitor::Visit(InvokeSlotMessage& message) {
  return object_->OnInvokeSlotMessage(message);
}

bool MessageVisitor::Visit(TimerMessage& message) {
  return object_->OnTimerMessage(message);
}

}