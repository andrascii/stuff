#pragma once

namespace mdo {

class TextMessage;
class LoopStarted;
class InvokeSlotMessage;
class TimerMessage;

class IMessageVisitor {
 public:
  virtual ~IMessageVisitor() = default;

  virtual bool Visit(TextMessage& message) = 0;
  virtual bool Visit(LoopStarted& message) = 0;
  virtual bool Visit(InvokeSlotMessage& message) = 0;
  virtual bool Visit(TimerMessage& message) = 0;
};

}