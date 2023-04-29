#include "application.h"

namespace eo {

void SigIntHandler(int signal) {
  switch (signal) {
    case SIGINT: {
      //printf("SIGINT, called from thread: %d", std::this_thread::get_id());
      the_main_thread.load(std::memory_order_relaxed)->Stop();
      break;
    }
  }
}

Application& Application::Instance() {
  static std::unique_ptr<Application> app = nullptr;

  if (!app) {
    app.reset(new Application);
  }

  return *app;
}

std::error_code Application::Exec() {
  ThreadAffinity()->Start();
  return {};
}

void Application::Quit() {
  ThreadAffinity()->Stop();
}

Application::Application() {
  std::signal(SIGINT, SigIntHandler);
}

}