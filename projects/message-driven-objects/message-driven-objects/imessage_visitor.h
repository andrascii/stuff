#pragma once

namespace mdo {

class InvokeSlotMessage;
class TimerMessage;
class BenchmarkMessage;
class TestMessage;

class IMessageVisitor {
 public:
  virtual ~IMessageVisitor() = default;

  virtual bool Visit(InvokeSlotMessage& message) = 0;
  virtual bool Visit(TimerMessage& message) = 0;
  virtual bool Visit(BenchmarkMessage& message) = 0;
  virtual bool Visit(TestMessage& message) = 0;
};

}// namespace mdo