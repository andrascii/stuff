#include "thread_pool.h"

using namespace std::literals;

namespace mdo {

ThreadPool::ThreadPool(uint32_t thread_count) {
  if (!thread_count) {
    throw std::logic_error{"thread count in thread pool cannot be equal to zero"};
  }

  for (uint32_t i = 0; i < thread_count; ++i) {
    threads_.emplace_back(Thread::Create(("pool#"s + std::to_string(i)).c_str()));
  }
}

void ThreadPool::Start() {
  LOG_TRACE("starting thread pool");

  for_each(threads_.begin(), threads_.end(), [](const auto& thread) {
    thread->Start();
  });

  LOG_TRACE("thread pool started");
}

void ThreadPool::Stop() {
  LOG_TRACE("stopping thread pool");

  for_each(threads_.begin(), threads_.end(), [](const auto& thread) {
    thread->Stop();
  });

  LOG_TRACE("thread pool stopped");
}

const std::shared_ptr<mdo::Thread>& ThreadPool::Thread() {
  const auto thread_id = round_robin_counter_.load(std::memory_order_relaxed) % threads_.size();
  round_robin_counter_.fetch_add(1, std::memory_order_relaxed);
  return threads_[thread_id];
}

}