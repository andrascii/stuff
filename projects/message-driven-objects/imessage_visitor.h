#pragma once

namespace mdo {

class InvokeSlotMessage;
class TimerMessage;

class IMessageVisitor {
 public:
  virtual ~IMessageVisitor() = default;

  virtual bool Visit(InvokeSlotMessage& message) = 0;
  virtual bool Visit(TimerMessage& message) = 0;
};

}// namespace mdo