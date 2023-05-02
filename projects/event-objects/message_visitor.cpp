#include "message_visitor.h"
#include "object.h"

namespace message_driven_objects {

MessageVisitor::MessageVisitor(Object* object)
    : object_{object} {}

bool MessageVisitor::Visit(const TextMessage& message) {
  return object_->OnTextMessage(message);
}

bool MessageVisitor::Visit(const LoopStarted& message) {
  return object_->OnLoopStarted(message);
}

}