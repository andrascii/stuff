#include "timer_service.h"
#include "timer_message.h"
#include "dispatcher.h"
#include "thread.h"

namespace mdo {

#if defined (_WIN32) || defined (_WIN64)

class TimerService::Impl {
 public:
  struct TimerContext {
    TimerContext(Object* object, TimerService::Impl* impl, const std::chrono::milliseconds& ms, int id, bool single_shot)
        : object{ object },
          timer_handle{ CreateWaitableTimer(NULL, 0, NULL) },
          impl{ impl },
          ms{ ms },
          id{ id },
          single_shot{ single_shot } {}

    Object* object;
    HANDLE timer_handle;
    TimerService::Impl* impl;
    std::chrono::milliseconds ms;
    int id;
    bool single_shot;
  };

  Impl()
      : kq_{ CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0) },
        evt_{ CreateEvent(NULL, 0, 0, NULL) } {
    if (!kq_ || !evt_) {
      std::terminate();
    }
  }

  Impl(Impl&& other) = default;
  Impl(const Impl& other) = delete;

  ~Impl() {
    CloseHandle(kq_);
    CloseHandle(evt_);
  }

  void Start() {
    iocp_f_ = std::async(std::launch::async, [this] {
      IocpThread();
    });

    timer_f_ = std::async(std::launch::async, [this] {
      TimerThread();
    });
  }

  void Stop() {
    request_interruption_.store(true, std::memory_order_relaxed);
    SetEvent(evt_);
    iocp_f_.get();
    timer_f_.get();
  }

  int AddTimer(Object* object, const std::chrono::milliseconds& ms, bool single_shot) {
    int timer_id = NextTimerId();
    auto context = std::make_unique<TimerContext>(object, this, ms, timer_id, single_shot);

    std::lock_guard<std::mutex> _{ mutex_ };
    to_add_.push_back(context.get());
    contexts_[timer_id] = std::move(context);

    return timer_id;
  }

  void RemoveTimer(int id) {
    std::lock_guard<std::mutex> _{ mutex_ };

    const auto it = contexts_.find(id);

    if (it == contexts_.end()) {
      return;
    }

    CancelWaitableTimer(it->second->timer_handle);
    CloseHandle(it->second->timer_handle);
  }

 private:
  static int NextTimerId() noexcept {
    static int timer_id = 0;
    return timer_id++;
  }

  static void TimerFunc(void* arg, DWORD dwTimerLowValue, DWORD dwTimerHighValue) {
    auto* context = static_cast<TimerContext*>(arg);

    if (!PostQueuedCompletionStatus(context->impl->kq_, 0, (ULONG_PTR)arg, NULL)) {
      std::terminate();
    }
  }

  void IocpThread() {
    // wait for incoming events from KQ and process them
    for (; !request_interruption_.load(std::memory_order_relaxed);) {
      OVERLAPPED_ENTRY events[1];
      ULONG n = 0;
      int timeout_ms = 100;
      BOOL ok = GetQueuedCompletionStatusEx(kq_, events, 1, &n, timeout_ms, 0);

      if (!ok) {
        continue;
      }

      auto* context = reinterpret_cast<TimerContext*>(events[0].lpCompletionKey);

      if (context->single_shot) {
        RemoveTimer(context->id);
      }

      printf("timer event\n");
    }
  }

  void TimerThread() {
    for (; !request_interruption_.load(std::memory_order_relaxed);) {
      {
        std::lock_guard<std::mutex> _{ mutex_ };

        for (const auto& context : to_add_) {
          long long due_ns100 = context->ms.count() * 1000 * -10;

          bool success = SetWaitableTimer(
            context->timer_handle,
            (LARGE_INTEGER*)&due_ns100,
            context->ms.count(),
            TimerFunc,
            context,
            1
          );

          if (!success) {
            std::terminate();
          }
        }

        to_add_.clear();
      }

      int r = WaitForSingleObjectEx(evt_, 0, /*alertable*/ 1);

      if (r == WAIT_OBJECT_0) {
        break;
      }
    }
  }

 private:
  HANDLE kq_;
  HANDLE evt_;

  std::future<void> iocp_f_;
  std::future<void> timer_f_;
  std::atomic_bool request_interruption_;

  std::mutex mutex_;
  std::deque<TimerContext*> to_add_;
  std::map<int, std::unique_ptr<TimerContext>> contexts_;
};

#elif defined (__APPLE__)

class TimerService::Impl {
 public:
  Impl()
      : kq_{kqueue()},
        events_count_{} {
    if (kq_ == -1) {
      SPDLOG_CRITICAL("error initializing kqueue");
      std::terminate();
    }
  }

  ~Impl() {
    if (managing_thread_) {
      managing_thread_->Stop();
    }

    close(kq_);
  }

  void Start() {
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

  void Stop() {
    std::scoped_lock _{mutex_};

    if (managing_thread_) {
      managing_thread_->Stop();
    }
  }

  int AddTimer(Object* object, const std::chrono::milliseconds& ms, bool single_shot) {
    std::scoped_lock _{mutex_};

    int timer_id = NextTimerId();

    uint16_t flags = EV_ADD | EV_ENABLE;

    if (single_shot) {
      flags |= EV_ONESHOT;
    }

    struct kevent evt{};
    EV_SET(&evt, timer_id, EVFILT_TIMER, flags, 0, ms.count(), object);

    ++events_count_;

    if (kevent(kq_, &evt, 1, nullptr, 0, nullptr)) {
      SPDLOG_CRITICAL("cannot add kevent with id: {}", timer_id);
      std::terminate();
    }

    return timer_id;
  }

  void RemoveTimer(int id) {
    std::scoped_lock _{mutex_};

    struct kevent evt{};
    EV_SET(&evt, id, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);

    if (kevent(kq_, &evt, 1, nullptr, 0, nullptr)) {
      SPDLOG_CRITICAL("cannot remove kevent with id: {}", id);
      std::terminate();
    }
  }

 private:
  static int NextTimerId() noexcept {
    static int timer_id = 0;
    return timer_id++;
  }

  void Run() {
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

 private:
  int kq_;
  mutable std::mutex mutex_;
  std::shared_ptr<Thread> managing_thread_;
  uint64_t events_count_;
};

#else

class TimerService::Impl {
 public:
  Impl() : kq_{ epoll_create(1) } {
    if (kq_ == -1) {
      SPDLOG_CRITICAL(strerror(errno));
      std::terminate();
    }
  }

  Impl(Impl&& other) = default;
  Impl(const Impl& other) = delete;

  ~Impl() {
    close(kq_);
  }

  void Start() {
    f_ = std::async(std::launch::async, [this]{
      TimerThread();
    });
  }

  void Stop() {
  }

  void Wait() {
    f_.wait();
  }

  int AddTimer(Object* object, const std::chrono::milliseconds& ms, bool single_shot) {
    // prepare timerfd-descriptor
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);

    if (timer_fd == -1) {
      SPDLOG_CRITICAL("timer register error: {}", strerror(errno));
    }

    assert(timer_fd != -1);

    // register timerfd in KQ
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = timer_fd;

    if (epoll_ctl(kq_, EPOLL_CTL_ADD, timer_fd, &event)) {
      SPDLOG_CRITICAL("cannot add epoll event: {}", strerror(errno));
      std::terminate();
    }

    // start periodic timer
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms);
    const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(ms);
    const auto remaining_nanoseconds = nanoseconds - seconds;

    struct itimerspec its{};
    memset(&its, 0, sizeof(its));
    its.it_value.tv_sec = seconds.count();
    its.it_value.tv_nsec = remaining_nanoseconds.count();

    if (!single_shot) {
      its.it_interval = its.it_value;
    }

    if (timerfd_settime(timer_fd, 0, &its, NULL)) {
      SPDLOG_CRITICAL("set timer error: {}", strerror(errno));
      std::terminate();
    }

    std::scoped_lock _{mutex_};
    timer_fds_.emplace(timer_fd, { single_shot, object });

    return timer_fd;
  }

  void RemoveTimer(int id) {
    std::scoped_lock _{mutex_};
    const auto it = timer_fds_.find(id);

    if (it == timer_fds_.end()) {
      return;
    }

    close(it->first);
    timer_fds_.erase(it);
  }

 private:
  void TimerThread() {
    std::vector<struct epoll_event> events;

    for (;;) {
      {
        std::scoped_lock _{mutex_};
        if (events.size() != timer_fds_.size()) {
          events.resize(timer_fds_.size());
        }
      }

      int n = epoll_wait(kq_, events.data(), events.size(), 0);

      for (int i = 0; i < n; ++i) {
        unsigned long long val;
        read(events[i].data.fd, &val, 8);

        if (events[i].events & (EPOLLIN | EPOLLERR)) {
          std::scoped_lock _{mutex_};

          const auto it = timer_fds_.find(events[i].data.fd);

          if (it == timer_fds_.end()) {
            SPDLOG_CRITICAL("occurred timer tick for unknown timer id: {}", events[i].data.fd);
            std::terminate();
          }

          SPDLOG_TRACE("dispatching timer tick for timer id: {}", events[i].data.fd);
          Dispatcher::Dispatch(std::make_shared<TimerMessage>(events[i].data.fd, nullptr, it->second.object));

          if (it->second.single_shot) {
            timer_fds_.erase(it);
          }
        }
      }

      std::this_thread::yield();
    }
  }

 private:
  struct TimerContext {
    bool single_shot;
    Object* object;
  };

  int kq_;
  std::future<void> f_;
  mutable std::mutex mutex_;
  std::map<int, TimerContext> timer_fds_;
};

#endif


TimerService::TimerService() : impl_{std::make_unique<Impl>()} {}

TimerService::~TimerService() = default;

void TimerService::Start() {
  return impl_->Start();
}

void TimerService::Stop() {
  return impl_->Stop();
}

int TimerService::AddTimer(NotNull<Object*> object, const milliseconds& ms, bool single_shot) {
  return impl_->AddTimer(object, ms, single_shot);
}

void TimerService::RemoveTimer(int id) {
  return impl_->RemoveTimer(id);
}

}