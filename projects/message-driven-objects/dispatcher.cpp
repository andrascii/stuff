#include "dispatcher.h"
#include "thread.h"

namespace mdo {

Dispatcher& Dispatcher::Instance() {
  static std::unique_ptr<Dispatcher> app = nullptr;

  if (!app) {
    app.reset(new Dispatcher);
  }

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