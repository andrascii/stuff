#include "message_base.h"
#include "object.h"

namespace mdo {

MessageBase::MessageBase(Object* sender, Object* receiver)
    : sender_{sender},
      receiver_{receiver} {}

Object* MessageBase::Sender() const noexcept {
  return sender_;
}

Object* MessageBase::Receiver() const noexcept {
  return receiver_;
}

}// namespace mdo