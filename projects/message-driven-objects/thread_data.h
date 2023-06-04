#pragma once

#include "message_queue.h"
#include "locked.h"

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

using ThreadDataPtr = std::shared_ptr<ThreadData>;

}