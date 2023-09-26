#include "command_line_arguments.h"
#include "config.h"
#include "producer.h"

/*
 * utility used to write data in to kafka:
 * 1. ./write-to-kafka -c config.json line-to-write1 line-to-write2
 * 2. cat file.txt | ./write-to-kafka -c config.json
 *
 * [-c;--config] parameter used to pass a path to config.json which specifies kafka settings.
 */

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

    Producer producer{config};

    std::string line;

    while (std::cin >> line) {
      producer.Write(line);
    }
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }
}