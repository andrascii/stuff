#pragma once

namespace mdo {

class Object;
class IMessageVisitor;

class IMessage {
 public:
  enum MessageType {
    kInvokeSlotMessage,
    kTimerMessage,
  };

  enum PriorityType {
    kLowestPriority,
    kLowPriority,
    kNormalPriority,
    kHighPriority,
    kHighestPriority,
    kTimeCriticalPriority,
  };

  virtual ~IMessage() = default;

  [[nodiscard]] virtual MessageType Type() const noexcept = 0;
  [[nodiscard]] virtual PriorityType Priority() const noexcept = 0;
  [[nodiscard]] virtual Object* Sender() const noexcept = 0;
  [[nodiscard]] virtual Object* Receiver() const noexcept = 0;
  virtual bool Accept(IMessageVisitor& visitor) noexcept = 0;

  friend bool operator<(const IMessage& lhs, const IMessage& rhs) {
    return lhs.Priority() < rhs.Priority();
  }
};

}// namespace mdo