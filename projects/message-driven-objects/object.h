#pragma once

#include "imessage.h"
#include "message_queue.h"

namespace message_driven_objects {

class Thread;
class TextMessage;
class LoopStarted;
class InvokeSlotMessage;

/*!

 Objects of this class form a tree: parent => child1, parent => child2 and so on.
 The most parent object of this class takes ownership of all his children.
 It means that when we delete the most parent object (the object that has children and has no parent)
 it automatically deletes all his children objects.
 It means that when you create a child object, you must do it allocating a memory on the heap.
 Consequently, if your Object has a parent you must not to delete it manually.

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

  explicit Object(Object* parent = nullptr);
  virtual ~Object();

  //!
  //! Returns a pointer to the parent object of this object. Function is reentrant.
  //!
  [[nodiscard]] Object* Parent() const noexcept;

  //!
  //! Sets the parent of this object. Function is reentrant.
  //!
  void SetParent(Object* parent);

  //!
  //! Returns a set of children of this object. Function is reentrant.
  //!
  const std::set<Object*>& Children() const noexcept;

  //!
  //! \param message is a message that will be broadcasted.
  //!
  //! It calls Object::OnMessage function for the most parent object.
  //! This lead to sending a message for all children tree including caller object.
  //!
  void BroadcastMessage(const std::shared_ptr<IMessage>& message);

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
  [[nodiscard]] message_driven_objects::Thread* Thread() const noexcept;

  //!
  //! \param thread sets the thread where this object will "live".
  //!
  void MoveToThread(message_driven_objects::Thread* thread) noexcept;

 protected:
  Object(message_driven_objects::Thread* thread, Object* parent);

  virtual void AddChild(Object* child) noexcept;
  virtual void RemoveChild(Object* child) noexcept;

  virtual bool OnTextMessage(TextMessage& message);
  virtual bool OnLoopStarted(LoopStarted& message);
  virtual bool OnInvokeSlotMessage(InvokeSlotMessage& message);

 private:
  Object* parent_;
  std::atomic<message_driven_objects::Thread*> thread_;
  std::set<Object*> children_;
};

}