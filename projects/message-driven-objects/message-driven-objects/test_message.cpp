#include "test_message.h"

namespace mdo {

TestMessage::TestMessage(const std::string& data, Object* sender, Object* receiver)
    : MessageBase(sender, receiver),
      data_{data} {}

const std::string& TestMessage::Data() const noexcept { return data_; }

}// namespace mdo