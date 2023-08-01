#include "message_queue.h"

namespace mdo {

MessageQueue::MessageQueue()
    : interrupt_{} {}

void MessageQueue::Push(Message&& message) {
  std::unique_lock _{mutex_};
  messages_.push_back(std::move(message));
  condition_.notify_all();

  LOG_TRACE("pushed message to queue '{}', queue size '{}'", (void*) this, messages_.size());
}

std::error_code MessageQueue::Poll(
  std::vector<Message>& messages,
  const std::chrono::seconds& timeout) noexcept {
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

  swap(messages, messages_);
  _.unlock();

  condition_.notify_all();

  return {};
}

void MessageQueue::SetInterruptFlag(bool value) noexcept {
  LOG_TRACE("set interrupt flag for queue '{}' to '{}'", (void*) this, value);

  std::lock_guard _{mutex_};
  interrupt_ = value;
  condition_.notify_all();
}

void MessageQueue::Clear() noexcept {
  LOG_TRACE("clearing queue '{}'", (void*) this);

  std::lock_guard _{mutex_};
  messages_.clear();
}

size_t MessageQueue::Size() const noexcept {
  std::lock_guard _{mutex_};
  return messages_.size();
}

}// namespace mdo