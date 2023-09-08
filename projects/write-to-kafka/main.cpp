#include "config.h"
#include "logger.h"
#include "producer.h"
#include "command_line_arguments.h"

int main(int argc, char** argv) {
  using namespace write_to_kafka;

  try {
    const auto expected_arguments = CommandLineArguments::Read(argc, argv, "write-to-kafka", "write-to-kafka");

    if (!expected_arguments) {
      std::cerr << "can't parse command line arguments\n";
      return EXIT_FAILURE;
    }

    const auto args = *expected_arguments;

    if (args->IsHelpRequested()) {
      std::cout << args->Help() << std::endl;
      return EXIT_SUCCESS;
    }

    Config config{args->ConfigPath()};

    if (config.QuietMode()) {
      DisableConsoleLogging();
    } else {
      EnableConsoleLogging();
    }

    Logger()->set_level(config.LogLevel());

    Producer producer{config};

    std::string line;

    while(std::cin >> line) {
      std::cout << "cin string: " << line << std::endl;
    }
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }
}