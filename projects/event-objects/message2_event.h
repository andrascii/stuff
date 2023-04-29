#pragma once

#include "ievent.h"

namespace eo {

class IEventVisitor;

class Message2Event : public IEvent {
 public:
  explicit Message2Event(Object* sender, Object* receiver);

  [[nodiscard]] EventType Type() const noexcept override;
  void Accept(IEventVisitor& visitor) const noexcept override;
  Object* Sender() const noexcept override;
  Object* Receiver() const noexcept override;

 private:
  Object* sender_;
  Object* receiver_;
};

}