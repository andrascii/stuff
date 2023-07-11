#pragma once

namespace mdo {

class Object;
class IMessageVisitor;

class IMessage {
 public:
  enum MessageType {
    kInvokeSlotMessage,
    kTimerMessage,
    kBenchmarkMessage,
    kTestMessage,
    kSetThreadNameMessage,
  };

  virtual ~IMessage() = default;

  [[nodiscard]] virtual MessageType Type() const noexcept = 0;
  [[nodiscard]] virtual Object* Sender() const noexcept = 0;
  [[nodiscard]] virtual Object* Receiver() const noexcept = 0;
  virtual bool Accept(IMessageVisitor& visitor) noexcept = 0;
};

}// namespace mdo