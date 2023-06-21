#include "timer_service.h"

#include "atomic_helpers.h"
#include "dispatcher.h"
#include "thread.h"
#include "timer_message.h"

namespace mdo {

#if defined(_WIN32) || defined(_WIN64)

class TimerService::Impl {
 public:
  static std::atomic<TimerService::Impl*> this_ptr;

  struct TimerContext {
    TimerContext(Object* object, TimerService::Impl* impl, const std::chrono::milliseconds& ms, int id, bool single_shot)
        : object{object},
          timer_handle{CreateWaitableTimer(NULL, 0, NULL)},
          impl{impl},
          ms{ms},
          id{id},
          single_shot{single_shot} {}

    Object* object;
    HANDLE timer_handle;
    TimerService::Impl* impl;
    std::chrono::milliseconds ms;
    int id;
    bool single_shot;
  };

  Impl()
      : kq_{CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)},
        evt_{CreateEvent(NULL, 0, 0, NULL)} {
    if (!kq_ || !evt_) {
      SPDLOG_CRITICAL("can't initialize I/O completion port");
      std::terminate();
    }

    iocp_thread_.reset(Thread::Create([this] {
      Thread::SetCurrentThreadName("IOCP");
      IocpThread();
    }));

    timer_thread_.reset(Thread::Create([this] {
      Thread::SetCurrentThreadName("TimerThread");
      TimerThread();
    }));

    StoreRelaxed(this_ptr, this);
  }

  Impl(Impl&& other) = default;
  Impl(const Impl& other) = delete;

  ~Impl() {
    CloseHandle(kq_);
    CloseHandle(evt_);
    StoreRelaxed<TimerService::Impl*>(this_ptr, nullptr);
  }

  void Start() {
    if (!iocp_thread_->IsRunning()) {
      iocp_thread_->Start();
      SPDLOG_TRACE("started IOCP thread");
    }

    if (!timer_thread_->IsRunning()) {
      timer_thread_->Start();
      SPDLOG_TRACE("started timer thread");
    }
  }

  void Stop() {
    SetEvent(evt_);
    iocp_thread_->Stop();
    timer_thread_->Stop();
  }

  int AddTimer(Object* object, const std::chrono::milliseconds& ms, bool single_shot) {
    int timer_id = NextTimerId();
    AddTimerImpl(timer_id, object, ms, single_shot);
    return timer_id;
  }

  void RemoveTimer(int id) {
    std::scoped_lock _{mutex_};

    const auto it = contexts_.find(id);

    if (it == contexts_.end()) {
      return;
    }

    CancelWaitableTimer(it->second->timer_handle);
    CloseHandle(it->second->timer_handle);
    contexts_.erase(it);
  }

  void ResetTimer(int id) {
    std::scoped_lock _{mutex_};

    const auto it = contexts_.find(id);

    if (it == contexts_.end()) {
      return;
    }

    const auto context = *it->second;

    RemoveTimer(id);

    AddTimerImpl(
      context.id,
      context.object,
      context.ms,
      context.single_shot);
  }

 private:
  static int NextTimerId() noexcept {
    static std::atomic<int> timer_id = 0;
    return timer_id.fetch_add(1, std::memory_order_relaxed);
  }

  void AddTimerImpl(int id, Object* object, const std::chrono::milliseconds& ms, bool single_shot) {
    auto context = std::make_unique<TimerContext>(object, this, ms, id, single_shot);

    std::scoped_lock _{mutex_};
    to_add_.push_back(context.get());
    contexts_[id] = std::move(context);
  }

  static void TimerFunc(void* arg, DWORD dwTimerLowValue, DWORD dwTimerHighValue) {
    UNREFERENCED_PARAMETER(dwTimerLowValue);
    UNREFERENCED_PARAMETER(dwTimerHighValue);

    if (!PostQueuedCompletionStatus(LoadRelaxed(this_ptr)->kq_, 0, (ULONG_PTR) arg, NULL)) {
      std::terminate();
    }
  }

  void IocpThread() {
    // wait for incoming events from KQ and process them
    for (; !Thread::Current()->IsInterruptionRequested();) {
      OVERLAPPED_ENTRY events[1];
      ULONG n = 0;
      BOOL ok = GetQueuedCompletionStatusEx(kq_, events, 1, &n, 0, 0);

      if (!ok) {
        continue;
      }

      int timer_id = static_cast<int>(events[0].lpCompletionKey);

      std::scoped_lock _{mutex_};
      const auto it = contexts_.find(timer_id);

      if (it == contexts_.end()) {
        SPDLOG_WARN("occurred timer tick for unknown timer id '{}', possibly it was deleted", timer_id);
        continue;
      }

      const auto& context = it->second;

      Dispatcher::Dispatch(
        std::make_shared<TimerMessage>(context->id, nullptr, context->object));

      if (context->single_shot) {
        RemoveTimer(context->id);
      }
    }
  }

#pragma warning(push)
#pragma warning(disable : 4312)

  void TimerThread() {
    for (; !Thread::Current()->IsInterruptionRequested();) {
      {
        std::scoped_lock _{mutex_};

        for (const auto& context : to_add_) {
          long long due_ns100 = context->ms.count() * 1000 * -10;

          bool success = SetWaitableTimer(
            context->timer_handle,
            (LARGE_INTEGER*) &due_ns100,
            (LONG) context->ms.count(),
            TimerFunc,
            reinterpret_cast<void*>(context->id),
            1);

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

#pragma warning(pop)

 private:
  HANDLE kq_;
  HANDLE evt_;

  std::unique_ptr<Thread> iocp_thread_;
  std::unique_ptr<Thread> timer_thread_;

  std::recursive_mutex mutex_;
  std::deque<TimerContext*> to_add_;
  std::map<int, std::unique_ptr<TimerContext>> contexts_;
};

std::atomic<TimerService::Impl*> TimerService::Impl::this_ptr;

#elif defined(__APPLE__)

class TimerService::Impl {
 public:
  Impl()
      : kq_{kqueue()},
        events_count_{} {
    if (kq_ == -1) {
      SPDLOG_CRITICAL("error initializing kqueue");
      std::terminate();
    }

    managing_thread_.reset(Thread::Create([this] {
      Run();
    }));
  }

  ~Impl() {
    managing_thread_->Stop();
    close(kq_);
  }

  void Start() {
    if (managing_thread_->IsRunning()) {
      return;
    }

    managing_thread_->Start();
  }

  void Stop() {
    managing_thread_->Stop();
  }

  int AddTimer(Object* object, const std::chrono::milliseconds& ms, bool single_shot) {
    int timer_id = NextTimerId();

    SetTimer(timer_id, object, ms, single_shot);

    {
      std::scoped_lock _{mutex_};

      contexts_[timer_id] = TimerContext{
        object,
        ms,
        single_shot};
    }

    events_count_.fetch_add(1, std::memory_order_relaxed);

    return timer_id;
  }

  void RemoveTimer(int id) {
    struct kevent evt {};
    EV_SET(&evt, id, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);

    if (kevent(kq_, &evt, 1, nullptr, 0, nullptr)) {
      SPDLOG_CRITICAL("cannot remove kevent with id: {}", id);
      std::terminate();
    }

    events_count_.fetch_sub(1, std::memory_order_relaxed);

    std::scoped_lock _{mutex_};
    contexts_.erase(id);
  }

  void ResetTimer(int id) {
    std::scoped_lock _{mutex_};
    const auto it = contexts_.find(id);

    if (it == contexts_.end()) {
      SPDLOG_WARN("resetting unknown timer id '{}'", id);
      return;
    }

    const auto& context = it->second;

    SetTimer(id, context.object, context.ms, context.single_shot);
  }

 private:
  static int NextTimerId() noexcept {
    static std::atomic<int> timer_id = 1;
    return timer_id.fetch_add(1, std::memory_order_relaxed);
  }

  void SetTimer(int id, Object* object, const std::chrono::milliseconds& ms, bool single_shot) const {
    uint16_t flags = EV_ADD | EV_ENABLE;

    if (single_shot) {
      flags |= EV_ONESHOT;
    }

    struct kevent evt {};
    EV_SET(&evt, id, EVFILT_TIMER, flags, 0, ms.count(), object);

    if (kevent(kq_, &evt, 1, nullptr, 0, nullptr)) {
      SPDLOG_CRITICAL("cannot add kevent with id: {}", id);
      std::terminate();
    }
  }

  void Run() {
    std::vector<struct kevent> events{LoadRelaxed(events_count_)};

    for (; !managing_thread_->IsInterruptionRequested();) {
      if (LoadRelaxed(events_count_) != events.size()) {
        events.resize(LoadRelaxed(events_count_));
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
          Dispatcher::Dispatch(std::make_shared<TimerMessage>(events[i].ident, nullptr, (Object*) events[i].udata));
        }
      }
    }
  }

 private:
  struct TimerContext {
    Object* object;
    std::chrono::milliseconds ms;
    bool single_shot;
  };

  int kq_;
  std::shared_ptr<Thread> managing_thread_;
  std::atomic<uint64_t> events_count_;

  mutable std::mutex mutex_;
  std::map<int, TimerContext> contexts_;
};

#else

class TimerService::Impl {
 public:
  Impl() : kq_{epoll_create(1)} {
    if (kq_ == -1) {
      SPDLOG_CRITICAL(strerror(errno));
      std::terminate();
    }
  }

  ~Impl() {
    close(kq_);
  }

  void Start() {
    if (f_.valid()) {
      return;
    }

    request_interruption_.store(false, std::memory_order_relaxed);

    f_ = std::async(std::launch::async, [this] {
      TimerThread();
    });
  }

  void Stop() {
    if (!f_.valid()) {
      return;
    }

    request_interruption_.store(true, std::memory_order_relaxed);

    f_.get();
  }

  int AddTimer(Object* object, const std::chrono::milliseconds& ms, bool single_shot) {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);

    if (timer_fd == -1) {
      SPDLOG_CRITICAL("timer register error: {}", strerror(errno));
    }

    assert(timer_fd != -1);

    // register timer_fd in KQ
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = timer_fd;

    if (epoll_ctl(kq_, EPOLL_CTL_ADD, timer_fd, &event)) {
      SPDLOG_CRITICAL("cannot add epoll event: {}", strerror(errno));
      std::terminate();
    }

    SetTimer(timer_fd, ms, single_shot);

    std::scoped_lock _{mutex_};
    timer_fds_.emplace(timer_fd, TimerContext{ms, single_shot, object});

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

  void ResetTimer(int id) {
    std::scoped_lock _{mutex_};
    const auto it = timer_fds_.find(id);

    if (it == end(timer_fds_)) {
      return;
    }

    const auto& context = it->second;

    SetTimer(id, context.ms, context.single_shot);
  }

 private:
  void SetTimer(int id, const std::chrono::milliseconds& ms, bool single_shot) const {
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms);
    const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(ms);
    const auto remaining_nanoseconds = nanoseconds - seconds;

    struct itimerspec its {};
    its.it_value.tv_sec = seconds.count();
    its.it_value.tv_nsec = remaining_nanoseconds.count();

    if (!single_shot) {
      its.it_interval = its.it_value;
    }

    if (timerfd_settime(id, 0, &its, NULL)) {
      SPDLOG_CRITICAL("set timer error: {}", strerror(errno));
      std::terminate();
    }
  }

  void TimerThread() {
    std::vector<struct epoll_event> events;

    for (; !request_interruption_.load(std::memory_order_relaxed);) {
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
          const auto extract_context = [this](const auto timer_id) -> std::optional<TimerContext> {
            std::scoped_lock _{mutex_};

            const auto it = timer_fds_.find(timer_id);

            if (it == timer_fds_.end()) {
              return std::nullopt;
            }

            TimerContext ctx = it->second;

            if (it->second.single_shot) {
              timer_fds_.erase(it);
            }

            return ctx;
          };

          const auto optional_context = extract_context(events[i].data.fd);

          if (!optional_context.has_value()) {
            SPDLOG_WARN("occurred timer tick for unknown timer id '{}', possibly it was deleted", events[i].data.fd);
            continue;
          }

          SPDLOG_TRACE("dispatching timer tick for timer id: {}", events[i].data.fd);
          Dispatcher::Dispatch(std::make_shared<TimerMessage>(events[i].data.fd, nullptr, it->second.object));
        }
      }

      std::this_thread::yield();
    }
  }

 private:
  struct TimerContext {
    std::chrono::milliseconds ms;
    bool single_shot;
    Object* object;
  };

  int kq_;
  std::future<void> f_;
  mutable std::mutex mutex_;
  std::map<int, TimerContext> timer_fds_;
  std::atomic_bool request_interruption_;
};

#endif

TimerService* TimerService::Instance() {
  struct NewOpEnabler : TimerService {
    NewOpEnabler() : TimerService() {}
  };

  static std::unique_ptr<NewOpEnabler> instance = std::make_unique<NewOpEnabler>();

  return instance.get();
}

TimerService::~TimerService() {
  impl_->Stop();
}

int TimerService::AddTimer(NotNull<Object*> object, const milliseconds& ms, bool single_shot) {
  return impl_->AddTimer(object, ms, single_shot);
}

void TimerService::RemoveTimer(int id) {
  return impl_->RemoveTimer(id);
}

void TimerService::ResetTimer(int id) {
  return impl_->ResetTimer(id);
}

TimerService::TimerService() : impl_{std::make_unique<Impl>()} {
  impl_->Start();
}

}// namespace mdo