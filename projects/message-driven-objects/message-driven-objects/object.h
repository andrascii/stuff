#pragma once

#include "locked.h"
#include "message.h"
#include "message_queue.h"

namespace mdo {

class Thread;
class InvokeSlotMessage;
class TimerMessage;
class BenchmarkMessage;
class TestMessage;

/*!

 Each object has thread affinity.
 "Object A lives in the thread T" - means that messages to A would be delivered
 (called function handler for this message) in the thread T.

 Assume that we have two objects: A and B.
 A lives in the main thread and B lives in the background thread.
 When A sends a message TextMessage to B, then this message would be posted to
 the thread T message queue. After that the thread T extracts the message and
 calls 'Object::OnMessage => Object::OnTextMessage' for object B.

 The developer must ensure that the object is deleted before the thread to be
 sure that Thread* thread_ in the Object's object is valid. Also developer
 usually should to stop the thread manually before the object that lives in this
 thread is deleted.

*/

class Object {
 public:
  Object();
  explicit Object(mdo::Thread* thread);

  virtual ~Object();

  //!
  //! Starts a timer and returns a timer identifier.
  //! A timer message will occur every interval milliseconds until KillTimer()
  //! is called. The virtual OnTimerMessage() function is called with the
  //! TimerMessage message parameter class when a timer message occurs.
  //! Reimplement this function to get timer messages.
  //!
  int StartTimer(const std::chrono::milliseconds& ms) noexcept;

  //!
  //! Kills the timer with timer identifier, id.
  //! The timer identifier is returned by StartTimer() when a timer message is
  //! started.
  //!
  void KillTimer(int id) noexcept;

  void ResetTimer(int id) const noexcept;

  //!
  //! \param message is a message that must be handled by this object
  //!
  //! Receives a message and calls corresponding handle function for it for
  //! this object.
  //!
  void OnMessage(Message& message);

  //!
  //! Returns the pointer to the thread where this object "lives".
  //!
  [[nodiscard]] mdo::Thread* Thread() const noexcept;

 protected:
  void SetThread(mdo::Thread* thread);

  virtual void OnTimerMessage(TimerMessage& message);
  virtual void OnBenchmarkMessage(BenchmarkMessage& message);
  virtual void OnTestMessage(TestMessage& message);

 private:
  void OnInvokeSlotMessage(InvokeSlotMessage& message);

 private:
  //
  // WARN: mutex must be recursive to avoid deadlock when handling SIGINT:
  // OS kernel can interrupt any thread at any time for handling the signal.
  // If this interruption occurs when someone already locked this mutex (for
  // example call of Dispatcher::Quit), then would be deadlock.
  //
  mutable std::recursive_mutex mutex_;
  mdo::Thread* thread_;
  std::set<int> timers_;
};

}// namespace mdo