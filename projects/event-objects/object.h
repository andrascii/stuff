#pragma once

#include "imessage.h"
#include "message_queue.h"

namespace eo {

class Thread;
class KafkaMessageNotification;
class DeliveryMessage;

//
// 1. each object must have thread affinity
// 2. the main thread is the thread where we create Dispatcher
//

/*!

 Objects of this class form a tree: parent => child1, parent => child2 and so on.
 The most parent object of this class takes ownership of all his children.
 It means that when we delete the most parent object (the object that has children and has no parent)
 it automatically deletes all his children objects.

*/

class Object {
 public:
  friend class MessageVisitor;

  explicit Object(Object* parent = nullptr);
  virtual ~Object();

  //!
  //! \return a pointer to the parent object of this object
  //!
  [[nodiscard]] Object* Parent() const noexcept;

  //!
  //! \param parent
  //!
  //! Sets the parent of this object
  //!
  void SetParent(Object* parent);

  //!
  //! \param message is a message that will be broadcasted
  //!
  //! It calls Object::OnMessage function for the most parent object.
  //! This lead to sending a message for all children tree.
  //!
  void BroadcastMessage(const std::shared_ptr<IMessage>& message);

  //!
  //! \param message is a message that must be handled of this object
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
  [[nodiscard]] eo::Thread* Thread() const noexcept;
  void MoveToThread(eo::Thread* thread) noexcept;

 protected:
  Object(eo::Thread* thread, Object* parent);

  virtual bool OnKafkaMessageNotification(const KafkaMessageNotification& message);
  virtual bool OnDeliveryMessage(const DeliveryMessage& message);

 private:
  void AddChild(Object* child) noexcept;

 private:
  Object* parent_;
  std::atomic<eo::Thread*> thread_;
  std::set<Object*> children_;
};

}