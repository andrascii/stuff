#pragma once

class IEventProcessor {
 public:
  using Callback = std::function<void(int)>;

  virtual ~IEventProcessor() = default;

  virtual bool RegisterEvent(
    int event_fd,
    int filter,
    Callback on_read,
    Callback on_write) = 0;

  virtual void ProcessEvents() = 0;
};
