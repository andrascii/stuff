#include "thread.h"

#include <utility>
#include "adopted_thread.h"
#include "object.h"
#include "loop_started.h"
#include "atomic_helpers.h"

#if defined(USE_WINDOWS_SET_THREAD_NAME_HACK)

namespace {

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;    // Must be 0x1000.
  LPCSTR szName;   // Pointer to name (in user addr space).
  DWORD dwThreadID;// Thread ID (-1=caller thread).
  DWORD dwFlags;   // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD thread_id, const std::string& thread_name) {
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = thread_name.data();
  info.dwThreadID = thread_id;
  info.dwFlags = 0;

  __try {
    const DWORD MS_VC_EXCEPTION = 0x406D1388;
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*) &info);
  } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

}// namespace

#endif

namespace {

void DeleteAdoptedMainThread();

}

namespace mdo {

std::atomic<Thread*> the_main_thread = nullptr;

ThreadDataPtr GetThreadData(Thread* thread) noexcept {
  return thread->data_;
}

Thread* Thread::Current() {
  if (current_thread_data) {
    return current_thread_data->thread;
  }

  current_thread_data = std::make_shared<ThreadData>();
  current_thread_data->is_adopted = true;
  StoreRelaxed(current_thread_data->id, std::this_thread::get_id());
  Thread* adopted_thread = new AdoptedThread{current_thread_data};
  current_thread_data->thread = adopted_thread;

  Thread* previous = the_main_thread.exchange(adopted_thread, std::memory_order_relaxed);

  if (previous) {
    while (!the_main_thread.compare_exchange_weak(
      adopted_thread,
      previous,
      std::memory_order_relaxed,
      std::memory_order_relaxed));
  } else {
    std::atexit(DeleteAdoptedMainThread);
  }

  return current_thread_data->thread;
}

void Thread::YieldCurrentThread() {
  std::this_thread::yield();
}

void Thread::Sleep(const std::chrono::milliseconds& ms) {
  std::this_thread::sleep_for(ms);
}

Thread::Thread(ThreadDataPtr data, Object* parent) : Thread{{}, std::move(data), parent} {}

Thread::~Thread() {
  StopImpl();

  const auto thread_name = name_.empty() ? ToString(std::this_thread::get_id()) : name_;

  SPDLOG_TRACE("thread {} destroyed", thread_name);

  data_->thread = nullptr;
}

const std::string& Thread::Name() const noexcept {
  return name_;
}

void Thread::SetName(const std::string& name) {
  name_ = name;

  if (data_->is_adopted) {
    SetCurrentThreadName(name_);
  }
}

void Thread::Start() {
  if (future_.valid()) {
    return;
  }

  future_ = std::async(std::launch::async, [this] {
    SetCurrentThreadName(name_);
    current_thread_data = data_;
    current_thread_data->id = std::this_thread::get_id();

    if (alternative_entry_point_) {
      alternative_entry_point_();
    } else {
      Run();
    }
  });
}

void Thread::Wait() const noexcept {
  if (!future_.valid()) {
  }

  future_.wait();
}

bool Thread::WaitFor(const std::chrono::milliseconds& ms) const noexcept {
  if (!future_.valid()) {
    return true;
  }

  const auto await_result = future_.wait_for(ms);

  if (await_result == std::future_status::deferred) {
    SPDLOG_CRITICAL("the thread function must not be in a deferred state");
    std::terminate();
  }

  if (await_result == std::future_status::timeout) {
    return false;
  }

  return true;
}

void Thread::Stop() {
  StopImpl();
}

bool Thread::IsRunning() const noexcept {
  return future_.valid() && future_.wait_for(0s) == std::future_status::timeout;
}

void Thread::RequestInterruption() const noexcept {
  StoreRelaxed(data_->interruption_requested, true);
}

bool Thread::IsInterruptionRequested() const noexcept {
  return LoadRelaxed(data_->interruption_requested);
}

void Thread::Run() {
  const auto tid = ToString(LoadRelaxed(current_thread_data->id));

  //
  // Sends LoopStartedMessage message to all objects which "lives" in this thread.
  // So the objects have an ability to find out when the loop is started and can initiate their job.
  //

  Thread* this_thread = LoadRelaxed(current_thread_data->thread);

  //
  // emit 'Started' signal
  //
  this_thread->Started();

  for (Object* object : this_thread->Children()) {
    if (object->Thread() != this_thread) {
      continue;
    }

    object->OnMessage(std::make_shared<LoopStartedMessage>(this_thread, object));
  }

  for (; !LoadRelaxed(current_thread_data->interruption_requested);) {
    std::shared_ptr<IMessage> message;
    const auto error = current_thread_data->queue.Poll(message, 1s);

    if (error == std::errc::interrupted) {
      SPDLOG_TRACE("the thread '{}' is interrupted", tid);
      break;
    }

    if (error == std::errc::timed_out) {
      SPDLOG_TRACE("the thread '{}' has no messages", tid);
      continue;
    }

    if (message) {
      SPDLOG_TRACE("the thread '{}' got a message", tid);
    }

    const auto receiver_thread = message->Receiver()->Thread();

    if (this_thread == receiver_thread) {
      message->Receiver()->OnMessage(message);
    } else {
      GetThreadData(receiver_thread)->queue.Push(message);
    }
  }

  //
  // emit 'Finished' signal
  //
  this_thread->Finished();
}

void Thread::SetCurrentThreadName(const std::string& name) noexcept {
#ifdef USE_WINDOWS_SET_THREAD_NAME_HACK
  SetThreadName(static_cast<DWORD>(-1), name);
#elif defined(__APPLE__)
  pthread_setname_np(name.data());
#else
  prctl(PR_SET_NAME, name.data(), 0, 0, 0);
#endif
}

void Thread::StopImpl() {
  if (!future_.valid()) {
    return;
  }

  const auto tid = ToString(data_->id);
  RequestInterruption();
  data_->queue.Exit();

  while (future_.wait_for(1s) == std::future_status::timeout) {
    SPDLOG_INFO("waiting for '{}' thread to stop", tid);
  }

  future_.get();

  SPDLOG_INFO("thread '{}' has stopped", tid);
}

void Thread::AddChild(Object* child) noexcept {
  Object::AddChild(child);

  if (IsRunning()) {
    current_thread_data->queue.Push(std::make_shared<LoopStartedMessage>(nullptr, child));
  }
}

Thread::Thread(std::function<void()> alternative_entry_point, ThreadDataPtr data, Object* parent)
    : Object{this, parent},
      Finished{this},
      Started{this},
      data_{std::move(data)},
      alternative_entry_point_{std::move(alternative_entry_point)} {
  if (!data_) {
    data_ = std::make_shared<ThreadData>();
  }

  data_->thread = this;
}

}

namespace {

void DeleteAdoptedMainThread() {
  using namespace mdo;
  delete LoadRelaxed(the_main_thread);
}

}