#include "thread_data.h"
#include "thread.h"
#include "atomic_helpers.h"

namespace message_driven_objects {

ThreadData::ThreadData()
    : id{std::this_thread::get_id()},
      thread{},
      interruption_requested{},
      is_adopted{} {
  SPDLOG_TRACE("thread data created");
}

ThreadData::~ThreadData() {
  if (is_adopted) {
    delete LoadRelaxed(thread);
  }

  SPDLOG_TRACE("thread data for thread {} destroyed", ToString(LoadRelaxed(id)));
}

}