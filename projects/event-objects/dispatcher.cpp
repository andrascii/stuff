#include "dispatcher.h"

#include "thread.h"

namespace eo {

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
  the_main_thread.load(std::memory_order_relaxed)->Stop();
}

void Dispatcher::Post(std::shared_ptr<IMessage> message) {
  ThreadData* data = GetThreadData(eo::Dispatcher::Instance().Thread());
  data->event_queue.Push(std::move(message));
}

}