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

Dispatcher::~Dispatcher() {
  std::cout << "current_thread_data ptr: " << current_thread_data.get() << std::endl;
  std::cout << "current_thread_data->Thread() ptr: " << current_thread_data->Thread() << std::endl;

  LOG_INFO("dispatcher destroyed in thread '{}'",
           current_thread_data->Thread()->Name());
}

std::error_code Dispatcher::Exec() {
  Thread()->SetName("dispatcher");
  Thread()->Start();
  return {};
}

void Dispatcher::Quit() {
  LOG_INFO("stopping dispatcher...");

  const auto thread = Instance().Thread();

  thread->Stop();

  //
  // WARN: mandatory call to ensure running tests
  //
  thread->Started.DisconnectAll();
}

void Dispatcher::Dispatch(Message&& message) {
  if (Instance().Thread()->IsInterruptionRequested()) {
    return;
  }

  const auto receiver_thread = std::visit(GetReceiver, message)->Thread();

  LOG_TRACE("dispatching message for thread '{}'", receiver_thread->Name());

  const auto data = GetThreadData(receiver_thread);

  data->Queue().Post(std::move(message));
}

}// namespace mdo