#pragma once

class IEvent;

class EventLoop {
 public:
  EventLoop();

  void Push(std::shared_ptr<IEvent> event);
  std::shared_ptr<IEvent> Pop() noexcept;
  void InterruptPopWaiting() noexcept;

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  std::queue<std::shared_ptr<IEvent>> events_;
  bool interrupt_;
};