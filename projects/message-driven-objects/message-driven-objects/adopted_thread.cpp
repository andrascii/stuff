#include "adopted_thread.h"

namespace mdo {

AdoptedThread::AdoptedThread(const std::shared_ptr<ThreadData>& data) : Thread{data} {}

void AdoptedThread::Start() {
  LOG_TRACE("starting '{}' thread", CurrentThreadId());
  Run();
}

void AdoptedThread::Stop() {
  GetThreadData(shared_from_this())->Queue().SetInterruptFlag(true);
  GetThreadData(shared_from_this())->Queue().Clear();
}

}// namespace mdo