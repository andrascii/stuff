#pragma once

#include "abstract_message.h"

namespace mdo {

class SetThreadNameMessage : public AbstractMessage {
 public:
  explicit SetThreadNameMessage(const std::string& name);

  const std::string& Name() const noexcept;

  [[nodiscard]] Object* Sender() const noexcept override;
  [[nodiscard]] Object* Receiver() const noexcept override;
  bool Accept(IMessageVisitor& visitor) noexcept override;

 private:
  std::string name_;
};

}
