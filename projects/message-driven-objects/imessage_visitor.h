#pragma once

namespace message_driven_objects {

class TextMessage;
class LoopStarted;
class InvokeSlotMessage;

class IMessageVisitor {
 public:
  virtual ~IMessageVisitor() = default;

  virtual bool Visit(TextMessage& message) = 0;
  virtual bool Visit(LoopStarted& message) = 0;
  virtual bool Visit(InvokeSlotMessage& message) = 0;
};

}