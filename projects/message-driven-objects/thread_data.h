#pragma once

#include "message_queue.h"

namespace message_driven_objects {

class Thread;

struct ThreadData {
  ThreadData();
  ~ThreadData();

  MessageQueue queue;
  std::atomic<std::thread::id> id;
  std::atomic<Thread*> thread;
  bool is_adopted;
};

using ThreadDataPtr = std::shared_ptr<ThreadData>;

}