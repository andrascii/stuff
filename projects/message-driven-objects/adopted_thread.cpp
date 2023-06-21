#include "adopted_thread.h"

namespace mdo {

AdoptedThread::AdoptedThread(const std::shared_ptr<ThreadData>& data) : Thread{data} {}

void AdoptedThread::Start() {
  Run();
}

void AdoptedThread::Stop() {
  GetThreadData(this)->queue.SetInterruptFlag(true);
}

}// namespace mdo