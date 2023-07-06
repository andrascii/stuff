#pragma once

#include "abstract_message.h"

namespace mdo {

class TimerMessage : public AbstractMessage {
 public:
  TimerMessage(int id, Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;

  int Id() const noexcept;

 private:
  int id_;
};

}// namespace mdo