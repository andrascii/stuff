#pragma once

#include "object.h"

namespace benchmarks {

using namespace mdo;

class TestMessageSender : public Object {
public:
  explicit TestMessageSender(
    const std::shared_ptr<mdo::Thread>& thread,
    size_t gen_msg_count,
    Object* receiver
  );

  void OnThreadStarted();

private:
  size_t gen_msg_count_;
  Object* receiver_;
};

}