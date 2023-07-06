#pragma once

#include "abstract_message.h"

namespace mdo {

class InvokeSlotMessage : public AbstractMessage {
 public:
  InvokeSlotMessage(const std::function<void()>& f, Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;
  void Invoke() const;

 private:
  std::function<void()> f_;
};

}// namespace mdo