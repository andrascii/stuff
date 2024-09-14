#pragma once

#include "ievent_processor.h"

class FreeBsdEventProcessor final : public IEventProcessor {
 public:
  FreeBsdEventProcessor();
  ~FreeBsdEventProcessor();

  bool RegisterEvent(
    int event_fd,
    int filter,
    Callback on_read,
    Callback on_write) override;

  void ProcessEvents() override;

 private:
  std::optional<Callback> ReadCallbackFor(int event_id) const;
  std::optional<Callback> WriteCallbackFor(int event_id) const;

 private:
  int kq_;
  mutable std::mutex mutex_;
  std::unordered_map<int, Callback> on_read_callbacks_;
  std::unordered_map<int, Callback> on_write_callbacks_;
  size_t event_count_;
};
