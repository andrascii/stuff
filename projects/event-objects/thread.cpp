#include "thread.h"
#include "adopted_thread.h"
#include "object.h"

namespace eo {

std::atomic<Thread*> the_main_thread = nullptr;

ThreadData* GetThreadData(Thread* thread) noexcept {
  return thread->data_;
}

Thread* Thread::Current() {
  if (current_thread_data) {
    return current_thread_data->thread;
  }

  current_thread_data = new ThreadData;
  current_thread_data->is_adopted = true;
  current_thread_data->id.store(std::this_thread::get_id(), std::memory_order_relaxed);
  Thread* adopted_thread = new AdoptedThread{current_thread_data};
  current_thread_data->thread = adopted_thread;

  Thread* previous = the_main_thread.exchange(adopted_thread, std::memory_order_relaxed);

  if (previous) {
    while (!the_main_thread.compare_exchange_weak(
      adopted_thread,
      previous,
      std::memory_order_relaxed,
      std::memory_order_relaxed));
  }

  return current_thread_data->thread;
}

Thread::Thread(ThreadData* data, Object* parent)
    : Object{this, parent},
      data_{data} {
  if (!data_) {
    data_ = new ThreadData;
  }

  data_->thread = this;
}

Thread::~Thread() {
  StopImpl();

  SPDLOG_TRACE("thread destroyed");

  data_->thread = nullptr;
  data_->Deref();
}

void Thread::Start() {
  if (future_.valid()) {
    return;
  }

  future_ = std::async(std::launch::async, [this] {
    current_thread_data = data_;
    current_thread_data->id = std::this_thread::get_id();
    ThreadEntryPoint();
  });
}

void Thread::Stop() {
  StopImpl();
}

void Thread::ThreadEntryPoint() {
  const auto tid = ToString(current_thread_data->id.load(std::memory_order_relaxed));

  for (;;) {
    std::shared_ptr<IMessage> event;
    const auto error = current_thread_data->event_queue.Poll(event, 1s);

    if (error == std::errc::interrupted) {
      SPDLOG_TRACE("the thread '{}' is interrupted", tid);
      break;
    }

    if (error == std::errc::timed_out) {
      SPDLOG_TRACE("the thread '{}' has no events", tid);
      continue;
    }

    if (event) {
      SPDLOG_TRACE("the thread '{}' got an event", tid);
    }

    if (event->Receiver()->Thread() == current_thread_data->thread.load(std::memory_order_relaxed)) {
      event->Receiver()->OnMessage(event);
    } else {
      GetThreadData(event->Receiver()->Thread())->event_queue.Push(event);
    }
  }
}

void Thread::StopImpl() {
  if (!future_.valid()) {
    return;
  }

  const auto tid = ToString(data_->id);
  data_->event_queue.Exit();

  while (future_.wait_for(1s) == std::future_status::timeout) {
    SPDLOG_INFO("waiting for '{}' thread to stop", tid);
  }

  future_.get();

  SPDLOG_INFO("thread '{}' has stopped", tid);
}

}