#include "utils.h"

#include "thread.h"

namespace mdo {

std::shared_ptr<Thread> Utils::CurrentThread() {
  return Thread::Current();
}

}// namespace mdo