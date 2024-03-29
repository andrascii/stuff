#pragma once

#include "object.h"
#include "test_message.h"
#include "measure.h"

namespace benchmarks {

using namespace mdo;

class TestMessageReceiver : public Object {
public:
  TestMessageReceiver(uint64_t iterations);
  ~TestMessageReceiver();

  void OnThreadStarted();

  size_t Counter() const noexcept;

protected:
  void OnTestMessage(TestMessage&) override;

private:
  size_t ctr_;
  uint64_t iterations_;
  Measure measure_;
};

}