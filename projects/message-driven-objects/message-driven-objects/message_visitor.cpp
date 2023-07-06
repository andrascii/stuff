#include "message_visitor.h"
#include "object.h"

namespace mdo {

MessageVisitor::MessageVisitor(Object* object)
    : object_{object} {}

bool MessageVisitor::Visit(InvokeSlotMessage& message) {
  return object_->OnInvokeSlotMessage(message);
}

bool MessageVisitor::Visit(TimerMessage& message) {
  return object_->OnTimerMessage(message);
}

bool MessageVisitor::Visit(BenchmarkMessage& message) {
  return object_->OnBenchmarkMessage(message);
}

bool MessageVisitor::Visit(TestMessage& message) {
  return object_->OnTestMessage(message);
}

}// namespace mdo