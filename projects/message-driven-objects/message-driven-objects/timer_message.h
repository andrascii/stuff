#pragma once

#include "message_base.h"

namespace mdo {

class TimerMessage : public MessageBase {
 public:
  TimerMessage(int id, Object* sender, Object* receiver);

  int Id() const noexcept;

 private:
  int id_;
};

}// namespace mdo