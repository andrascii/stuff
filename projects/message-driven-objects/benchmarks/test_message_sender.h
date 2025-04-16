#pragma once

#include "measure.h"
#include "object.h"

namespace benchmarks {

using namespace mdo;

class TestMessageSender : public Object {
 public:
  TestMessageSender(
    mdo::Thread* thread,
    size_t gen_msg_count,
    Object* receiver);

  ~TestMessageSender();

  void OnThreadStarted();

 private:
  size_t gen_msg_count_;
  Object* receiver_;
  Measure measure_;
};

}// namespace benchmarks
