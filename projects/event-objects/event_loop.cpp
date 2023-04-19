#include "event_loop.h"
#include "ievent.h"

EventLoop::EventLoop() : interrupt_{} {}

void EventLoop::Push(std::shared_ptr<IEvent> event) {
  std::lock_guard _{ mutex_ };
  events_.push(std::move(event));
  condition_.notify_all();
}

std::shared_ptr<IEvent> EventLoop::Pop() noexcept {
  std::unique_lock _{ mutex_ };
  condition_.wait(_, [this] {
    return interrupt_ || !events_.empty();
  });

  if (interrupt_) {
    return nullptr;
  }

  std::shared_ptr<IEvent> event = std::move_if_noexcept(events_.back());
  events_.pop();

  return event;
}

void EventLoop::InterruptPopWaiting() noexcept {
  std::lock_guard _{ mutex_ };
  interrupt_ = true;
  condition_.notify_all();
}