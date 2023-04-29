#include "application.h"

namespace eo {

void SigIntHandler(int signal) {
  switch (signal) {
    case SIGINT: {
      std::cout << "SIGINT, called from thread: " << std::this_thread::get_id() << std::endl;
      the_main_thread.load(std::memory_order_relaxed)->Stop();
      break;
    }
  }
}

Application::Application(Object* parent) : Object{parent} {
  std::signal(SIGINT, SigIntHandler);
}

std::error_code Application::Exec() {
  ThreadAffinity()->Start();
  return {};
}

void Application::Quit() {
  ThreadAffinity()->Stop();
}

}