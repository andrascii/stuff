#include "message_queue.h"

#include "imessage.h"

namespace mdo {

MessageQueue::MessageQueue() : interrupt_{} {}

void MessageQueue::Push(std::shared_ptr<IMessage> message) {
  std::lock_guard _{mutex_};
  messages_.push(std::move(message));
  condition_.notify_all();
}

std::error_code MessageQueue::Poll(
  std::shared_ptr<IMessage>& message,
  const std::chrono::seconds& timeout
) noexcept {
  std::unique_lock _{mutex_};

  const auto has_event_or_interrupted = [this] {
    return interrupt_ || !messages_.empty();
  };

  if (!condition_.wait_for(_, timeout, has_event_or_interrupted)) {
    return std::make_error_code(std::errc::timed_out);
  }

  if (interrupt_) {
    return std::make_error_code(std::errc::interrupted);
  }

  message = std::move_if_noexcept(messages_.front());
  messages_.pop();

  return {};
}

void MessageQueue::SetInterruptFlag(bool value) noexcept {
  std::lock_guard _{mutex_};
  interrupt_ = value;

  if (interrupt_) {
    condition_.notify_all();
  } else {
    while(!messages_.empty()) {
      messages_.pop();
    }
  }
}

}