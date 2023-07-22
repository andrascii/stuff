#pragma once

#include "message_base.h"

namespace mdo {

class BenchmarkMessage : public MessageBase {
 public:
  BenchmarkMessage(Object* sender, Object* receiver);
};

}// namespace mdo