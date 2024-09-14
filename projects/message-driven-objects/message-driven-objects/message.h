#pragma once

#include "benchmark_message.h"
#include "invoke_slot_message.h"
#include "set_thread_name_message.h"
#include "test_message.h"
#include "timer_message.h"

namespace mdo {

class Thread;

using Message =
  std::variant<std::monostate, InvokeSlotMessage, TestMessage, BenchmarkMessage, SetThreadNameMessage, TimerMessage>;

const auto GetReceiver = [](auto&& msg) -> Object* {
  using T = std::decay_t<decltype(msg)>;

  if constexpr (std::is_same_v<std::monostate, T>) {
    abort();
  } else {
    return msg.Receiver();
  }
};

const auto GetSender = [](auto&& msg) -> Object* {
  using T = std::decay_t<decltype(msg)>;

  if constexpr (std::is_same_v<std::monostate, T>) {
    abort();
    return nullptr;
  } else {
    return msg.Sender();
  }
};

}// namespace mdo