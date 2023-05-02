#pragma once

namespace message_driven_objects {

class TextMessage;
class LoopStarted;

class IMessageVisitor {
 public:
  virtual ~IMessageVisitor() = default;

  virtual bool Visit(const TextMessage& message) = 0;
  virtual bool Visit(const LoopStarted& message) = 0;
};

}