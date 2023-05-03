#pragma once

#include "imessage_visitor.h"

namespace message_driven_objects {

class Object;

class MessageVisitor : public IMessageVisitor {
 public:
  explicit MessageVisitor(Object* object);

  bool Visit(const TextMessage& message) override;
  bool Visit(const LoopStarted& message) override;

 private:
  Object* object_;
};

}