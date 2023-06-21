#pragma once

#include "locked.h"
#include "message_queue.h"

namespace mdo {

class Thread;

struct ThreadData {
  ThreadData();
  ~ThreadData();

  MessageQueue queue;
  Locked<std::thread::id> id;
  Locked<Thread*> thread;
  Locked<bool> interruption_requested;
  Locked<bool> is_adopted;
};

}// namespace mdo