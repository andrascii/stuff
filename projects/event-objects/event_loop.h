#pragma once

namespace eo {

class IEvent;

class EventLoop {
 public:
  EventLoop();

  void Push(std::shared_ptr<IEvent> event);
  std::error_code Poll(std::shared_ptr<IEvent>& event, const std::chrono::seconds& timeout = 0s) noexcept;

  void Exit() noexcept;

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  std::queue<std::shared_ptr<IEvent>> events_;
  bool interrupt_;
};

}