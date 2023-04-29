#pragma once

#include "ievent.h"
#include "event_loop.h"
#include "thread.h"

namespace eo {

class Message1Event;
class Message2Event;

//
// 1. each object must have thread affinity
// 2. the main thread is the thread where we create Application
//

class Object {
 public:
  friend class EventVisitor;

  explicit Object(Object* parent = nullptr);
  virtual ~Object();

  [[nodiscard]] Object* Parent() const noexcept;
  void SetParent(Object* parent);

  void BroadcastEvent(const std::shared_ptr<IEvent>& event);
  bool Event(const std::shared_ptr<IEvent>& event);

  Thread* ThreadAffinity() const noexcept;
  void MoveToThread(Thread* thread) noexcept;

 protected:
  virtual bool OnMessage1Event(const Message1Event& event);
  virtual bool OnMessage2Event(const Message2Event& event);

 private:
  void AddChild(Object* child) noexcept;

 private:
  Object* parent_;
  std::set<Object*> children_;
  Thread* thread_;
};

}