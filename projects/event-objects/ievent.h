#pragma once

namespace eo {

class Object;
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
  virtual Object* Sender() const noexcept = 0;
  virtual Object* Receiver() const noexcept = 0;
};

}