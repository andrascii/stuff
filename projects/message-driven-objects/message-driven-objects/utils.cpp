#include "utils.h"

#include "thread.h"

namespace mdo {

Thread* Utils::CurrentThread() {
  return Thread::Current();
}

}// namespace mdo