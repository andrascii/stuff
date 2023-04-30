#include "adopted_thread.h"

namespace eo {

AdoptedThread::AdoptedThread(ThreadData* data) : Thread{data} {}

void AdoptedThread::Start() {
  ThreadEntryPoint();
}

void AdoptedThread::Stop() {
  GetThreadData(this)->event_queue.Exit();
}

}