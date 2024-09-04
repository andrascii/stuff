#include "free_bsd_event_processor.h"

#include <sys/event.h>
#include <unistd.h>

#include <utility>

FreeBsdEventProcessor::FreeBsdEventProcessor() : event_count_{} {
  kq_ = kqueue();

  if (kq_ == -1) {
    throw std::runtime_error{"Failed to create kqueue"};
  }
}

FreeBsdEventProcessor::~FreeBsdEventProcessor() { close(kq_); }

bool FreeBsdEventProcessor::RegisterEvent(
  int event_fd,
  int filter,
  Callback on_read,
  Callback on_write) {
  struct kevent event;
  EV_SET(&event, event_fd, filter, EV_ADD, 0, 0, nullptr);

  if (kevent(kq_, &event, 1, nullptr, 0, nullptr) == -1) {
    std::cerr << "Failed to register event with kqueue" << std::endl;
    return false;
  }

  std::scoped_lock _{mutex_};
  on_read_callbacks_[event_fd] = std::move(on_read);
  on_write_callbacks_[event_fd] = std::move(on_write);
  ++event_count_;
  return true;
}

void FreeBsdEventProcessor::ProcessEvents() {
  std::vector<struct kevent> events;

  if (events.size() != event_count_) {
    events.resize(event_count_);
  }

  const int num_events =
    kevent(kq_, nullptr, 0, events.data(), events.size(), nullptr);

  if (num_events == -1) {
    std::cerr << "error in kevent: " << strerror(errno) << std::endl;
    return;
  }

  for (int i = 0; i < num_events; ++i) {
    struct kevent& event = events[i];

    std::optional<Callback> optional_callback;

    if (event.filter == EVFILT_READ) {
      optional_callback = ReadCallbackFor(event.ident);
    } else if (event.filter == EVFILT_WRITE) {
      optional_callback = WriteCallbackFor(event.ident);
    }

    if (!optional_callback) {
      std::cerr
        << "not found callback for event id '"
        << event.ident
        << "' for filter '"
        << event.filter
        << "'"
        << std::endl;

      // remove event from kqueue
      return;
    }

    (*optional_callback)(event.ident);
  }
}

std::optional<FreeBsdEventProcessor::Callback> FreeBsdEventProcessor::ReadCallbackFor(int event_id) const {
  std::scoped_lock _{mutex_};
  const auto it = on_read_callbacks_.find(event_id);

  if (it == on_read_callbacks_.end()) {
    return std::nullopt;
  }

  return it->second;
}

std::optional<FreeBsdEventProcessor::Callback> FreeBsdEventProcessor::WriteCallbackFor(int event_id) const {
  std::scoped_lock _{mutex_};
  const auto it = on_write_callbacks_.find(event_id);

  if (it == on_write_callbacks_.end()) {
    return std::nullopt;
  }

  return it->second;
}
