#pragma once

#include "imessage_visitor.h"

namespace eo {

class Object;

class MessageVisitor : public IMessageVisitor {
 public:
  explicit MessageVisitor(Object* object);

  bool Visit(const KafkaMessageNotification& message) override;
  bool Visit(const DeliveryMessage& message) override;

 private:
  Object* object_;
};

}