#pragma once

#include "message_base.h"

namespace mdo {

class SetThreadNameMessage : public MessageBase {
 public:
  explicit SetThreadNameMessage(const std::string& name);

  const std::string& Name() const noexcept;

 private:
  std::string name_;
};

}// namespace mdo
