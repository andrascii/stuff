#pragma once

#include "message_base.h"

namespace mdo {

class InvokeSlotMessage : public MessageBase {
 public:
  InvokeSlotMessage(const std::function<void()>& f, Object* sender, Object* receiver);

  void Invoke() const;

 private:
  std::function<void()> f_;
};

}// namespace mdo
