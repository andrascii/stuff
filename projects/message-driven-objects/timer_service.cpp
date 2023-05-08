#include "timer_service.h"
#include "timer_message.h"
#include "dispatcher.h"
#include "thread.h"

namespace mdo {

TimerService::TimerService() : kq_{kqueue()} {
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

  timer_contexts_[timer_id] = {
    object
  };

  struct kevent evt{};
  EV_SET(&evt, timer_id, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, ms.count(), nullptr);

  kevents_.push_back(evt);

  //
  // TODO: kevents_ is a vector that can overwrite himself and pointer will be invalid
  //
  if (kevent(kq_, kevents_.data(), kevents_.size(), nullptr, 0, nullptr)) {
    SPDLOG_CRITICAL("cannot add kevent");
    std::terminate();
  }

  return timer_id;
}

void TimerService::RemoveTimer(int id) {
  std::scoped_lock _{mutex_};


}

int TimerService::NextTimerId() const noexcept {
  static int timer_id = 0;
  return timer_id++;
}

void TimerService::Run() {
  std::vector<struct kevent> events{kevents_.size()};

  for (; !managing_thread_->IsInterruptionRequested();) {
    if (kevents_.size() != events.size()) {
      events.resize(kevents_.size());
    }

    int n = kevent(kq_, nullptr, 0, events.data(), events.size(), nullptr);

    if (n <= 0) {
      continue;
    }

    for (int i = 0; i < n; ++i) {
      if (events[i].filter == EVFILT_TIMER) {
        const auto it = timer_contexts_.find(events[i].ident);

        if (it == timer_contexts_.end()) {
          SPDLOG_CRITICAL("occurred event for unknown timer id {}", events[i].ident);
          std::terminate();
        }

        const auto& [id, context] = *it;

        SPDLOG_TRACE("dispatching timer tick for timer id: {}", id);
        Dispatcher::Dispatch(std::make_shared<TimerMessage>(id, nullptr, context.object));
      }
    }
  }
}

}