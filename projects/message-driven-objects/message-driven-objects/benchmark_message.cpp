#include "benchmark_message.h"

namespace mdo {

BenchmarkMessage::BenchmarkMessage(Object* sender, Object* receiver)
    : MessageBase{sender, receiver} {}

}// namespace mdo
