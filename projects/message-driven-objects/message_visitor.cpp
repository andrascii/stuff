#include "message_visitor.h"
#include "object.h"

namespace message_driven_objects {

MessageVisitor::MessageVisitor(Object* object)
    : object_{object} {}

bool MessageVisitor::Visit(TextMessage& message) {
  return object_->OnTextMessage(message);
}

bool MessageVisitor::Visit(LoopStarted& message) {
  return object_->OnLoopStarted(message);
}

bool MessageVisitor::Visit(InvokeSlotMessage& message) {
  return object_->OnInvokeSlotMessage(message);
}

}