#include "utils.h"

#include "thread.h"

namespace mdo {

Thread* Utils::CurrentThread() {
  return Thread::Current();
}

bool Utils::IsThreadRunning(Thread* thread) {
  return thread->IsRunning();
}

}// namespace mdo