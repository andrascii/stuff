#include "dispatcher.h"
#include "thread.h"

namespace mdo {

Dispatcher& Dispatcher::Instance() {
  struct NewOpEnabler : Dispatcher {
    NewOpEnabler() : Dispatcher() {}
  };

  static std::unique_ptr<Dispatcher> app = std::make_unique<NewOpEnabler>();

  return *app;
}

std::error_code Dispatcher::Exec() {
  Thread()->Start();
  return {};
}

void Dispatcher::Quit() {
  Instance().Thread()->Stop();
}

void Dispatcher::Dispatch(std::shared_ptr<IMessage> message) {
  ThreadDataPtr data = GetThreadData(Instance().Thread());
  data->queue.Push(std::move(message));
}

}