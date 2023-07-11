#include "set_thread_name_message.h"

namespace mdo {

SetThreadNameMessage::SetThreadNameMessage(const std::string& name)
    : AbstractMessage{kSetThreadNameMessage, nullptr, nullptr},
      name_{name} {}

const std::string& SetThreadNameMessage::Name() const noexcept {
  return name_;
}

Object* SetThreadNameMessage::Sender() const noexcept {
  LOG_CRITICAL("this method must never be called");
  std::abort();
  return nullptr;
}

Object* SetThreadNameMessage::Receiver() const noexcept {
  LOG_CRITICAL("this method must never be called");
  std::abort();
  return nullptr;
}

bool SetThreadNameMessage::Accept(IMessageVisitor&) noexcept {
  LOG_CRITICAL("this method must never be called");
  std::abort();
  return false;
}

}