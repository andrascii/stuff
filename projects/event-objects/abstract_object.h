#pragma once

#include "ievent.h"
#include "event_loop.h"

class Message1Event;
class Message2Event;

class AbstractObject {
 public:
  friend class EventVisitor;

  AbstractObject();
  virtual ~AbstractObject();

  void ConnectEventReceiver(AbstractObject* object, IEvent::EventType type);
  void PostEvent(const std::shared_ptr<IEvent>& event);
  void BroadcastEvent(const std::shared_ptr<IEvent>& event);
  void Exec();

 protected:
  virtual bool OnMessage1Event(const Message1Event& event);
  virtual bool OnMessage2Event(const Message2Event& event);

 private:
  std::multimap<IEvent::EventType, AbstractObject*> objects_;
  EventLoop loop_;
};