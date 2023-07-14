#pragma once

#include "abstract_message.h"

namespace mdo {

class TestMessage : public AbstractMessage {
 public:
  TestMessage(const std::string& data, Object* sender, Object* receiver);

  bool Accept(IMessageVisitor& visitor) noexcept override;

  const std::string& Data() const noexcept;

 private:
  std::string data_;
};

}// namespace mdo
