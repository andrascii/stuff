#pragma once

#include "object.h"
#include "event_loop.h"

//
// 1. the thread where we create Application object is the main thread
//

namespace eo {

class Application : public Object {
 public:
  Application(Object* parent = nullptr);

  std::error_code Exec();
  void Quit();
};

}