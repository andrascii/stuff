#pragma once

namespace mdo {

class Object;

class MessageBase {
 public:
  MessageBase(Object* sender, Object* receiver);

  [[nodiscard]] Object* Sender() const noexcept;
  [[nodiscard]] Object* Receiver() const noexcept;

 private:
  Object* sender_;
  Object* receiver_;
};

}// namespace mdo