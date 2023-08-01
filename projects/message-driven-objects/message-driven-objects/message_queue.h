#pragma once

#include "message.h"

namespace mdo {

class MessageQueue {
 public:
  MessageQueue();

  void Push(Message&& message);

  //!
  //! Extracts a message from queue and assigns 'message' argument to extracted value.
  //!
  //! Returns error code describing the result of calling, error code could be:
  //!     - std::errc::timed_out (if reached 'timeout' value).
  //!     - std::errc::interrupted (if SetInterruptFlag function was called with first parameter is true).
  //!     - no error (if all is ok, in this case 'message' argument would contain a pointer to extracted message).
  //!
  std::error_code Poll(
    std::vector<Message>& messages,
    const std::chrono::seconds& timeout = 0s) noexcept;

  void SetInterruptFlag(bool value) noexcept;

  void Clear() noexcept;

  size_t Size() const noexcept;

 private:
  //
  // WARN: mutex must be recursive to avoid deadlock when handling SIGINT:
  // OS kernel can interrupt any thread at any time for handling the signal.
  // If this interruption occurs when someone already locked this mutex (for example call of Dispatcher::Quit), then would be deadlock.
  //
  mutable std::recursive_mutex mutex_;
  std::condition_variable_any condition_;
  std::vector<Message> messages_;
  bool interrupt_;
};

}// namespace mdo