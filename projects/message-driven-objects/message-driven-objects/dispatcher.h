#pragma once

#include "message_queue.h"
#include "object.h"

namespace mdo {

class Dispatcher : public Object {
 public:
  static Dispatcher& Instance();

  ~Dispatcher();

  std::error_code Exec();

  static void Quit();
  static void Dispatch(Message&& message);

 private:
  Dispatcher() = default;
};

}// namespace mdo