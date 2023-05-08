#include "timer_service.h"
#include "timer_message.h"
#include "dispatcher.h"
#include "thread.h"

namespace mdo {

TimerService::TimerService()
    : kq_{kqueue()},
      events_count_{} {
  if (kq_ == -1) {
    SPDLOG_CRITICAL("error initializing kqueue");
    std::terminate();
  }
}

TimerService::~TimerService() {
  if (managing_thread_) {
    managing_thread_->Stop();
  }

  close(kq_);
}

void TimerService::Start() {
  std::scoped_lock _{mutex_};

  if (managing_thread_) {
    if (managing_thread_->IsRunning()) {
      return;
    }

    managing_thread_->Start();
    return;
  }

  managing_thread_.reset(Thread::Create([this] {
    Run();
  }));

  managing_thread_->Start();
}

void TimerService::Stop() {
  std::scoped_lock _{mutex_};

  if (managing_thread_) {
    managing_thread_->Stop();
  }
}

int TimerService::AddTimer(NotNull<Object*> object, const std::chrono::milliseconds& ms) {
  std::scoped_lock _{mutex_};

  int timer_id = NextTimerId();

  struct kevent evt{};
  EV_SET(&evt, timer_id, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, ms.count(), object);

  ++events_count_;

  if (kevent(kq_, &evt, 1, nullptr, 0, nullptr)) {
    SPDLOG_CRITICAL("cannot add kevent with id: {}", timer_id);
    std::terminate();
  }

  return timer_id;
}

void TimerService::RemoveTimer(int id) {
  std::scoped_lock _{mutex_};

  struct kevent evt{};
  EV_SET(&evt, id, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);

  if (kevent(kq_, &evt, 1, nullptr, 0, nullptr)) {
    SPDLOG_CRITICAL("cannot remove kevent with id: {}", id);
    std::terminate();
  }
}

int TimerService::NextTimerId() noexcept {
  static int timer_id = 0;
  return timer_id++;
}

void TimerService::Run() {
  std::vector<struct kevent> events{events_count_};

  for (; !managing_thread_->IsInterruptionRequested();) {
    if (events_count_ != events.size()) {
      events.resize(events_count_);
    }

    timespec timeout{};
    timeout.tv_sec = 1;

    int n = kevent(kq_, nullptr, 0, events.data(), events.size(), &timeout);

    if (n <= 0) {
      continue;
    }

    for (int i = 0; i < n; ++i) {
      if (events[i].filter == EVFILT_TIMER) {
        SPDLOG_TRACE("dispatching timer tick for timer id: {}", events[i].ident);
        Dispatcher::Dispatch(std::make_shared<TimerMessage>(events[i].ident, nullptr, (Object*)events[i].udata));
      }
    }
  }
}

}