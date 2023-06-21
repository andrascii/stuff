#include "thread_data.h"

#include "atomic_helpers.h"
#include "thread.h"

namespace mdo {

ThreadData::ThreadData()
    : id{std::this_thread::get_id()},
      thread{},
      interruption_requested{},
      is_adopted{} {
  SPDLOG_TRACE("thread data created");
}

ThreadData::~ThreadData() {
  if (*is_adopted) {
    delete *thread;
  }

  SPDLOG_TRACE("thread data for thread {} destroyed", ToString(*id));
}

}// namespace mdo