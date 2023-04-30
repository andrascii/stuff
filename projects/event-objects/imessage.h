#pragma once

namespace eo {

class Object;
class IMessageVisitor;

class IMessage {
 public:
  enum MessageType {
    kKafkaMessageNotification,
    kDeliveryMessage,
  };

  virtual ~IMessage() = default;

  [[nodiscard]] virtual MessageType Type() const noexcept = 0;
  [[nodiscard]] virtual Object* Sender() const noexcept = 0;
  [[nodiscard]] virtual Object* Receiver() const noexcept = 0;
  virtual bool Accept(IMessageVisitor& visitor) const noexcept = 0;
};

}