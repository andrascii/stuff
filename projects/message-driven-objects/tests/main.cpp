#include "logger.h"

int main(int argc, char** argv) {
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "-l") != 0) {
      continue;
    }

    mdo::EnableConsoleLogging();
    mdo::Logger()->set_level(spdlog::level::trace);
    break;
  }

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}