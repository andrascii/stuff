#include "thread.h"
#include "adopted_thread.h"

namespace {

template <typename T>
std::string ToString(const T& data) {
  std::stringstream ss;
  ss << data;
  return ss.str();
}

}

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

Thread::Thread(ThreadData* data) : data_{data} {
  std::cout << "thread created\n";

  if (!data_) {
    data_.store(new ThreadData, std::memory_order_relaxed);
  }

  data_.load(std::memory_order_relaxed)->thread = this;
}

Thread::~Thread() {
  std::cout << "thread destroyed\n";

  data_.load(std::memory_order_relaxed)->thread = nullptr;
  data_.load(std::memory_order_relaxed)->Deref();
}

void Thread::Start() {
  if (future_.valid()) {
    return;
  }

  future_ = std::async(std::launch::async, [this] {
    current_thread_data = data_.load(std::memory_order_relaxed);
    current_thread_data->id = std::this_thread::get_id();
    Run();
  });
}

void Thread::Stop() {
  if (!future_.valid()) {
    return;
  }

  const auto tid = ToString(data_.load(std::memory_order_relaxed)->id);
  data_.load(std::memory_order_relaxed)->event_loop.Exit();

  while (future_.wait_for(1s) == std::future_status::timeout) {
    std::cout << "waiting for " << tid << " thread to stop\n";
  }

  future_.get();
  std::cout << "thread " << tid << " has stopped\n";
}

void Thread::Run() {
  const auto tid = ToString(current_thread_data->id.load(std::memory_order_relaxed));

  for (;;) {
    std::shared_ptr<IEvent> event;
    const auto error = current_thread_data->event_loop.Poll(event, 1s);

    if (error == std::errc::interrupted) {
      std::cout << "the thread '" << tid << "' is interrupted\n";
      break;
    }

    if (error == std::errc::timed_out) {
      std::cout << "the thread '" << tid << "' has no events\n";
      continue;
    }

    if (event) {
      std::cout << "the thread '" << tid << "' got an event\n";
    }
  }
}

}