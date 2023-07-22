#pragma once

#include "message_base.h"

namespace mdo {

class TestMessage : public MessageBase {
 public:
  TestMessage(const std::string& data, Object* sender, Object* receiver);

  const std::string& Data() const noexcept;

 private:
  std::string data_;
};

}// namespace mdo
