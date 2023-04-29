#include "adopted_thread.h"

namespace eo {

AdoptedThread::AdoptedThread(ThreadData* data) : Thread{data} {}

void AdoptedThread::Start() {
  Run();
}

void AdoptedThread::Stop() {
  GetThreadData(this)->event_loop.Exit();
}

}