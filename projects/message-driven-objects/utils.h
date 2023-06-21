#pragma once

namespace mdo {

class Thread;

class Utils final {
 public:
  //
  // This function is the Thread::Current() redirection.
  // Used to compare calling thread and callee thread when invoking signal.
  //
  // This redirection is needed because thread.h uses header with Signal<T> implementation
  // and the signal itself requires including thread.h that leads to recursive include.
  //
  static Thread* CurrentThread();
};

}// namespace mdo