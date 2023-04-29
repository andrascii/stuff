#include "event_loop.h"
#include "ievent.h"

namespace eo {

EventLoop::EventLoop() : interrupt_{} {}

void EventLoop::Push(std::shared_ptr<IEvent> event) {
  std::lock_guard _{mutex_};
  events_.push(std::move(event));
  condition_.notify_all();
}

std::error_code EventLoop::Poll(std::shared_ptr<IEvent>& event, const std::chrono::seconds& timeout) noexcept {
  std::unique_lock _{mutex_};

  const auto has_event_or_interrupted = [this] {
    return interrupt_ || !events_.empty();
  };

  if (!condition_.wait_for(_, timeout, has_event_or_interrupted)) {
    return std::make_error_code(std::errc::timed_out);
  }

  if (interrupt_) {
    return std::make_error_code(std::errc::interrupted);
  }

  event = std::move_if_noexcept(events_.back());
  events_.pop();

  return {};
}

void EventLoop::Exit() noexcept {
  std::lock_guard _{mutex_};
  interrupt_ = true;
  condition_.notify_all();
}

}