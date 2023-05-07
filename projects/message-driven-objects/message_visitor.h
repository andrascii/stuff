#pragma once

#include "imessage_visitor.h"

namespace message_driven_objects {

class Object;

class MessageVisitor : public IMessageVisitor {
 public:
  explicit MessageVisitor(Object* object);

  bool Visit(TextMessage& message) override;
  bool Visit(LoopStarted& message) override;
  bool Visit(InvokeSlotMessage& message) override;

 private:
  Object* object_;
};

}