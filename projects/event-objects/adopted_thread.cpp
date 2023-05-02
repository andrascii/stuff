#include "adopted_thread.h"

namespace message_driven_objects {

AdoptedThread::AdoptedThread(const ThreadDataPtr& data) : Thread{data} {}

void AdoptedThread::Start() {
  Run();
}

void AdoptedThread::Stop() {
  GetThreadData(this)->queue.Exit();
}

}