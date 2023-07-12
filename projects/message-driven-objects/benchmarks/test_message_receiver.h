#pragma once

#include "object.h"
#include "test_message.h"

namespace benchmarks {

using namespace mdo;

class TestMessageReceiver : public Object {
public:
  TestMessageReceiver();

  void OnThreadStarted();

  size_t Counter() const noexcept;

protected:
  bool OnTestMessage(TestMessage&) override;

private:
  size_t ctr_;
};

}