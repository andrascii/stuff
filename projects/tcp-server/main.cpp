#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "free_bsd_event_processor.h"
#include "runnable.h"
#include "tcp_socket.h"

template <typename T, typename E = std::error_code>
using Expected = tl::expected<T, E>;

template <typename E = std::error_code>
using Unexpected = tl::unexpected<E>;

struct CommandLineArguments {
  std::string host;
  unsigned short port;
  bool help_requested{};
};

template <typename T>
T GetOptionValue(
  const cxxopts::ParseResult& result,
  const std::string& option) {
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

Expected<CommandLineArguments> ReadCommandLineArguments(int argc, char** argv) {
  try {
    const auto kPortOption = "p,port"s;
    const auto kHostOption = "h,host"s;
    const auto kHelpOption = "help"s;

    cxxopts::Options options{"TCP Server", "Asynchronous TCP Server"};

    auto opts_adder = options.add_options();
    opts_adder(kPortOption, "TCP Server port to listen", cxxopts::value<unsigned short>());
    opts_adder(kHostOption, "TCP Server ip address to listen", cxxopts::value<std::string>());
    opts_adder(kHelpOption, "Prints this message");

    const auto result = options.parse(argc, argv);
    const auto help_requested = result.count(kHelpOption) > 0;

    const auto host = GetOptionValue<std::string>(result, kHostOption);
    const auto port = GetOptionValue<unsigned short>(result, kPortOption);

    return CommandLineArguments{host, port, help_requested};
  } catch (const std::exception& ex) {
    std::cerr
      << "error reading command line arguments: "
      << ex.what()
      << std::endl;

    return Unexpected<>{std::make_error_code(std::errc::invalid_argument)};
  }
}

int main(int argc, char** argv) {
  const auto expected_cmd_line_arguments =
    ReadCommandLineArguments(argc, argv);

  if (!expected_cmd_line_arguments) {
    return EXIT_FAILURE;
  }

  const auto cmd_line_arguments = *expected_cmd_line_arguments;

  const auto ep = std::make_shared<FreeBsdEventProcessor>();

  const auto run_loop = [ep] {
    return ep->ProcessEvents();
  };

  Runnable runnable{run_loop};
  runnable.Run();

  TcpSocket socket{ep};
  socket.Bind("0.0.0.0", 12345);
  socket.Listen({});

  std::this_thread::sleep_for(300s);

  /*sockaddr_in server_addr{};
  // sockaddr_in client_addr{};

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd == -1) {
    std::cerr << "socket creation failed...\n";
    return EXIT_FAILURE;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(cmd_line_arguments.port);

  if (bind(socket_fd, (sockaddr*) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "socket bind failed...\n";
    close(socket_fd);
    return EXIT_FAILURE;
  }*/
}
