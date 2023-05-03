#include "thread_data.h"
#include "thread.h"
#include "atomic_helpers.h"

namespace message_driven_objects {

ThreadData::ThreadData()
    : id{std::this_thread::get_id()},
      thread{},
      is_adopted{} {
  SPDLOG_INFO("thread data created");
}
ThreadData::~ThreadData() {
  if (is_adopted) {
    delete LoadRelaxed(thread);
  }

  SPDLOG_INFO("thread data destroyed");
}

}