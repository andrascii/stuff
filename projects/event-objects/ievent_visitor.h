#pragma once

namespace eo {

class Message1Event;
class Message2Event;

class IEventVisitor {
 public:
  virtual ~IEventVisitor() = default;

  virtual void Visit(const Message1Event& event) = 0;
  virtual void Visit(const Message2Event& event) = 0;
};

}