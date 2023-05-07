#pragma once

#include "message_queue.h"
#include "object.h"

//
// 1. the thread where we create Dispatch object is the main thread
//

namespace message_driven_objects {

class Dispatcher : public Object {
 public:
  static Dispatcher& Instance();

  std::error_code Exec();

  static void Quit();
  static void Dispatch(std::shared_ptr<IMessage> message);

 private:
  Dispatcher() = default;
};

}