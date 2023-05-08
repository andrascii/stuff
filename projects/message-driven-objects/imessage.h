#pragma once

namespace mdo {

class Object;
class IMessageVisitor;

class IMessage {
 public:
  enum MessageType {
    kTextMessage,
    kLoopStarted,
    kInvokeSlotMessage,
    kTimerMessage,
  };

  virtual ~IMessage() = default;

  [[nodiscard]] virtual MessageType Type() const noexcept = 0;
  [[nodiscard]] virtual Object* Sender() const noexcept = 0;
  [[nodiscard]] virtual Object* Receiver() const noexcept = 0;
  virtual bool Accept(IMessageVisitor& visitor) noexcept = 0;
};

}