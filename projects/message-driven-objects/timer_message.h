#pragma once

#include "abstract_message.h"

namespace mdo {

class TimerMessage : public AbstractMessage {
 public:
  TimerMessage(int id, Object* sender, Object* receiver, PriorityType priority = kTimeCriticalPriority);

  bool Accept(IMessageVisitor& visitor) noexcept override;

  int Id() const noexcept;

 private:
  int id_;
};

}