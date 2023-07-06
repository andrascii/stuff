#include "test_message.h"
#include "imessage_visitor.h"

namespace mdo {

TestMessage::TestMessage(const std::string& data, Object* sender, Object* receiver)
    : AbstractMessage(kTestMessage, sender, receiver),
      data_{data} {}

bool TestMessage::Accept(IMessageVisitor& visitor) noexcept {
  return visitor.Visit(*this);
}

const std::string& TestMessage::Data() const noexcept {
  return data_;
}

}