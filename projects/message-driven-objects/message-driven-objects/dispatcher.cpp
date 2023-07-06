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
  Thread()->SetName("dispatcher");
  Thread()->Start();
  return {};
}

void Dispatcher::Quit() {
  const auto thread = Instance().Thread();
  const auto thread_data = GetThreadData(thread);

  thread->Stop();

  //
  // WARN: mandatory call to ensure running tests
  //
  thread->Started.DisconnectAll();
}

void Dispatcher::Dispatch(std::shared_ptr<IMessage> message) {
  const auto data = GetThreadData(message->Receiver()->Thread());
  data->Queue().Push(std::move(message));
}

}// namespace mdo