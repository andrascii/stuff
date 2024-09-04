#include "tcp_socket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <sys/event.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <utility>

TcpSocket::TcpSocket(std::shared_ptr<IEventProcessor> event_processor)
    : descriptor_{-1},
      event_processor_{std::move(event_processor)} {
  EnsureSocketCreated();

  if (descriptor_ == -1) {
    throw std::runtime_error{
      fmt::format("socket creation failed: {}", strerror(errno))};
  }

  event_processor_->RegisterEvent(
    descriptor_,
    EVFILT_READ | EVFILT_WRITE,
    [this](int id) { OnRead(id); },
    [this](int id) { OnWrite(id); });
}

int TcpSocket::NativeHandle() const noexcept { return descriptor_; }

bool TcpSocket::Bind(const std::string& host, unsigned short port) noexcept {
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(inet_addr(host.data()));
  address.sin_port = htons(port);
  return Bind(address);
}

bool TcpSocket::Bind(const sockaddr_in& address) noexcept {
  const auto result = bind(descriptor_, (sockaddr*) &address, sizeof(address));

  if (result != 0) {
    std::cerr << "socket bind failed: " << strerror(errno) << std::endl;
    Close();
    return false;
  }

  return true;
}

bool TcpSocket::Listen() noexcept {
  const auto result = listen(descriptor_, SOMAXCONN);

  if (result) {
    std::cerr << errno << ": " << strerror(errno) << std::endl;
    return false;
  }

  return true;
}

void TcpSocket::Close() noexcept {
  close(descriptor_);
  EnsureSocketCreated();
}

void TcpSocket::Shutdown() noexcept { shutdown(descriptor_, SHUT_RDWR); }

void TcpSocket::OnRead(int descriptor) noexcept {
  int client_socket = -1;
  sockaddr client_address{};
  socklen_t size{};

  if (descriptor == descriptor_) {
    client_socket = accept(descriptor_, &client_address, &size);

    if (client_socket < 0) {
      std::cerr << fmt::format("error accepting connection: {}", strerror(errno));
      return;
    }
  }

  char buf[4096];
  int n = 0;

  while ((n = read(client_socket, buf, sizeof buf)) > 0) {
    std::cout << "read " << n << " bytes: " << buf << std::endl;
    write(client_socket, buf, n);
  }
}

void TcpSocket::OnWrite(int descriptor) noexcept {
  (void) descriptor;
  std::cout << "on write called\n";
}

void TcpSocket::EnsureSocketCreated() {
  if (descriptor_ != -1) {
    return;
  }

  descriptor_ = socket(AF_INET, SOCK_STREAM, 0);

  int flags = fcntl(descriptor_, F_GETFL, 0);
  fcntl(descriptor_, F_SETFL, flags | O_NONBLOCK);
}
