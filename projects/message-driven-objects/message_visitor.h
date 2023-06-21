#pragma once

#include "imessage_visitor.h"

namespace mdo {

class Object;

class MessageVisitor : public IMessageVisitor {
 public:
  explicit MessageVisitor(Object* object);

  bool Visit(InvokeSlotMessage& message) override;
  bool Visit(TimerMessage& message) override;

 private:
  Object* object_;
};

}// namespace mdo