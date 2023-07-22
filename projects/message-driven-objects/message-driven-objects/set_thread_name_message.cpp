#include "set_thread_name_message.h"

namespace mdo {

SetThreadNameMessage::SetThreadNameMessage(const std::string& name)
    : MessageBase{nullptr, nullptr},
      name_{name} {}

const std::string& SetThreadNameMessage::Name() const noexcept {
  return name_;
}

}// namespace mdo