#pragma once

#include "ievent_visitor.h"

class AbstractObject;

class EventVisitor : public IEventVisitor {
 public:
  explicit EventVisitor(AbstractObject* object);

  void Visit(const Message1Event& event) override;
  void Visit(const Message2Event& event) override;

 private:
  AbstractObject* object_;
};