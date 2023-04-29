#pragma once

#include "ievent_visitor.h"

namespace eo {

class Object;

class EventVisitor : public IEventVisitor {
 public:
  explicit EventVisitor(Object* object);

  void Visit(const Message1Event& event) override;
  void Visit(const Message2Event& event) override;

 private:
  Object* object_;
};

}