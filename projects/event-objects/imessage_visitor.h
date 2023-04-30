#pragma once

namespace eo {

class KafkaMessageNotification;
class DeliveryMessage;

class IMessageVisitor {
 public:
  virtual ~IMessageVisitor() = default;

  virtual bool Visit(const KafkaMessageNotification& message) = 0;
  virtual bool Visit(const DeliveryMessage& message) = 0;
};

}