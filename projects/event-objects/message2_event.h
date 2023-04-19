#pragma once

#include "ievent.h"

class IEventVisitor;

class Message2Event : public IEvent {
 public:
  [[nodiscard]] EventType Type() const noexcept override;
  void Accept(IEventVisitor& visitor) const noexcept override;
};