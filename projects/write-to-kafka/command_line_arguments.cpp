#include "command_line_arguments.h"

#include "errors.h"
#include "types.h"

namespace {

constexpr auto kConfigOption{"c,config"};
constexpr auto kConfigOptionDescription{"The path to the JSON config file"};

constexpr auto kHelpOption{"h,help"};
constexpr auto kHelpOptionDescription{"Prints this help message"};

template <typename T>
T GetOptionValue(const cxxopts::ParseResult& result, const std::string& option) {
  const auto comma_position = option.find(',');
  const auto is_composed_option = comma_position != std::string::npos;

  if (!is_composed_option) {
    return result[option].as<T>();
  }

  try {
    return result[option.substr(0, comma_position)].as<T>();
  } catch (const std::exception&) {
    return result[option.substr(comma_position + 1)].as<T>();
  }
}

}// namespace

namespace write_to_kafka {

Expected<std::shared_ptr<CommandLineArguments>> CommandLineArguments::Read(
  int argc,
  char** argv,
  const std::string& app_name,
  const std::string& app_description) {
  struct MakeSharedEnabler : public CommandLineArguments {
    MakeSharedEnabler(
      std::filesystem::path json_config_path,
      bool help_requested,
      std::string help,
      std::vector<std::string> unmatched)
        : CommandLineArguments{std::move(json_config_path), help_requested, std::move(help), std::move(unmatched)} {}
  };

  try {
    cxxopts::Options options{app_name, app_description};

    options.add_options()(kConfigOption, kConfigOptionDescription, cxxopts::value<std::filesystem::path>())(kHelpOption, kHelpOptionDescription);

    options.allow_unrecognised_options();

    const auto result = options.parse(argc, argv);
    const auto help_requested = result.count(kHelpOption) > 0;
    const auto json_config_path = GetOptionValue<std::filesystem::path>(result, kConfigOption);

    return std::make_shared<MakeSharedEnabler>(json_config_path, help_requested, options.help(), result.unmatched());
  } catch (const std::exception& ex) {
    std::cerr << fmt::format("error initializing command line arguments: {}", ex.what());
    return Unexpected<>{MakeErrorCode(Error::kCommandLineParsingError)};
  }
}

std::filesystem::path CommandLineArguments::ConfigPath() const noexcept { return json_config_path_; }

CommandLineArguments::CommandLineArguments(
  std::filesystem::path json_config_path,
  bool help_requested,
  std::string help,
  std::vector<std::string> unmatched)
    : json_config_path_{std::move(json_config_path)},
      help_requested_{help_requested},
      help_{std::move(help)},
      unmatched_{std::move(unmatched)} {}

bool CommandLineArguments::IsHelpRequested() const noexcept {
  return help_requested_;
}

const std::string& CommandLineArguments::Help() const {
  return help_;
}

const std::vector<std::string>& CommandLineArguments::Unmatched() const noexcept {
  return unmatched_;
}

}// namespace write_to_kafka
