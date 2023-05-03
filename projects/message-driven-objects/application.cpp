#include "application.h"

#include "dispatcher.h"
#include "text_message.h"
#include "thread.h"

namespace {

void SigIntHandler(int signal) {
  if (signal == SIGINT) {
    message_driven_objects::Dispatcher::Quit();
  }
}

}

namespace message_driven_objects {

Application::Application()
    : Object{&Dispatcher::Instance()},
      counter_{} {
  std::signal(SIGINT, SigIntHandler);

  start_ = system_clock::now();
}

Application::~Application() {
  auto end = system_clock::now();
  SPDLOG_INFO("received {} messages, elapsed time: {} milliseconds", counter_, duration_cast<milliseconds>(end - start_).count());
}

std::error_code Application::Exec() {
  return Dispatcher::Instance().Exec();
}

bool Application::OnTextMessage(const TextMessage& message) {
  SPDLOG_INFO("{}: received message: {}", Thread()->Name(), message.Message());

  if (message.Sender()) {
    Dispatcher::Post(std::make_shared<TextMessage>("Hello from Application object", this, message.Sender()));
  }

  ++counter_;
  return true;
}

}