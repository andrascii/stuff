#include "benchmark_message.h"

#include "imessage_visitor.h"

namespace mdo {

BenchmarkMessage::BenchmarkMessage(Object* sender, Object* receiver)
    : AbstractMessage{kBenchmarkMessage, sender, receiver} {}

bool BenchmarkMessage::Accept(IMessageVisitor& visitor) noexcept {
  return visitor.Visit(*this);
}

}// namespace mdo
