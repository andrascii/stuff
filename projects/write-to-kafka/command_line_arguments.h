#pragma once

#include "types.h"

namespace write_to_kafka {

class CommandLineArguments {
 public:
  static Expected<std::shared_ptr<CommandLineArguments>>
  Read(int argc, char** argv, const std::string& app_name, const std::string& app_description);

  //
  // DB settings
  //
  std::filesystem::path ConfigPath() const noexcept;
  bool IsHelpRequested() const noexcept;
  const std::string& Help() const;
  const std::vector<std::string>& Unmatched() const noexcept;

 private:
  explicit CommandLineArguments(std::filesystem::path json_config_path,
                                bool help_requested,
                                std::string help,
                                std::vector<std::string> unmatched);

 private:
  std::filesystem::path json_config_path_;
  bool help_requested_;
  std::string help_;
  std::vector<std::string> unmatched_;
};

}// namespace write_to_kafka
