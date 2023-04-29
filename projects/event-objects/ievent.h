#pragma once

namespace eo {

class IEventVisitor;

class IEvent {
 public:
  enum EventType {
    kMessage1,
    kMessage2,
  };

  virtual ~IEvent() = default;

  [[nodiscard]] virtual EventType Type() const noexcept = 0;
  virtual void Accept(IEventVisitor& visitor) const noexcept = 0;
};

}