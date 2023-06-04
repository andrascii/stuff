#pragma once

#include "imessage.h"
#include "message_queue.h"
#include "locked.h"

namespace mdo {

class Thread;
class InvokeSlotMessage;
class TimerMessage;

/*!

 Each object has thread affinity.
 "Object O lives in thread T" - means that messages to O would be delivered (called function handler for this message) in the thread T.
 Assume that we have two objects: Client1 and Client2.
 Client1 lives in the main thread and Client2 lives in the background thread.
 When Client1 sends a message TextMessage to Client2, then this message would be posted to the thread T message queue.
 After that the thread T extracts the message and calls 'Object::OnMessage => Object::OnTextMessage' for object Client2.

*/

class Object {
 public:
  friend class MessageVisitor;

  Object();
  Object(mdo::Thread* thread);

  virtual ~Object();

  //!
  //! Starts a timer and returns a timer identifier.
  //! A timer message will occur every interval milliseconds until KillTimer() is called.
  //! The virtual OnTimerMessage() function is called with the TimerMessage message parameter class when a timer message occurs.
  //! Reimplement this function to get timer messages.
  //!
  int StartTimer(const std::chrono::milliseconds& ms) noexcept;

  //!
  //! Kills the timer with timer identifier, id.
  //! The timer identifier is returned by StartTimer() when a timer message is started.
  //!
  void KillTimer(int id) noexcept;

  //!
  //! \param message is a message that must be handled by this object
  //!
  //! Receives a message and calls corresponding handle function for it for this object.
  //! Then by default it sends a message to all children of this object.
  //! When some child returns true as a result of handling message it means that that child accepted the message.
  //! When a message is accepted the delivery a message process to children objects interrupts and considered completed.
  //!
  //! If no one child returned true then a message would be delivered to all children objects.
  //!
  bool OnMessage(const std::shared_ptr<IMessage>& message);

  //!
  //! Returns the pointer to the thread where this object "lives".
  //!
  [[nodiscard]] mdo::Thread* Thread() const noexcept;

  //!
  //! \param thread sets the thread where this object will "live".
  //!
  void MoveToThread(mdo::Thread* thread) noexcept;

 protected:
  //
  // This function does not intended to be a virtual function
  //
  bool OnInvokeSlotMessage(InvokeSlotMessage& message);
  virtual bool OnTimerMessage(TimerMessage& message);

 private:
  Locked<mdo::Thread*> thread_;
  Locked<std::set<int>> timers_;
};

}