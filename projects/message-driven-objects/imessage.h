#pragma once

namespace message_driven_objects {

class Object;
class IMessageVisitor;

class IMessage {
 public:
  enum MessageType {
    kTextMessage,
    kLoopStarted,
    kInvokeSlotMessage,
  };

  virtual ~IMessage() = default;

  [[nodiscard]] virtual MessageType Type() const noexcept = 0;
  [[nodiscard]] virtual Object* Sender() const noexcept = 0;
  [[nodiscard]] virtual Object* Receiver() const noexcept = 0;
  virtual bool Accept(IMessageVisitor& visitor) noexcept = 0;
};

}