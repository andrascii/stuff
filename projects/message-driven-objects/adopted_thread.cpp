#include "adopted_thread.h"

namespace mdo {

AdoptedThread::AdoptedThread(const ThreadDataPtr& data) : Thread{data} {}

void AdoptedThread::Start() {
  Run();
}

void AdoptedThread::Stop() {
  GetThreadData(this)->queue.SetInterruptFlag(true);
}

}