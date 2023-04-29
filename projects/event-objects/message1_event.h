#pragma once

#include "ievent.h"

namespace eo {

class IEventVisitor;

class Message1Event : public IEvent {
 public:
  [[nodiscard]] EventType Type() const noexcept override;
  void Accept(IEventVisitor& visitor) const noexcept override;
};

}