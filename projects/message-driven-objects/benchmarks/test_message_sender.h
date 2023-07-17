#pragma once

#include "object.h"
#include "measure.h"

namespace benchmarks {

using namespace mdo;

class TestMessageSender : public Object {
public:
  explicit TestMessageSender(
    const std::shared_ptr<IExecutionPolicy>& execution_policy,
    size_t gen_msg_count,
    Object* receiver
  );

  void OnThreadStarted();

private:
  size_t gen_msg_count_;
  Object* receiver_;
  Measure measure_;
};

}