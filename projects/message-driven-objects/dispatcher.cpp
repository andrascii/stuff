#include "dispatcher.h"
#include "thread.h"
#include "atomic_helpers.h"

namespace message_driven_objects {

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
  LoadRelaxed(the_main_thread)->Stop();
}

void Dispatcher::Post(std::shared_ptr<IMessage> message) {
  ThreadDataPtr data = GetThreadData(Dispatcher::Instance().Thread());
  data->queue.Push(std::move(message));
}

}