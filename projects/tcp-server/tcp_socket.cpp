#include "tcp_socket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

TcpSocket::TcpSocket(const std::shared_ptr<IEventProcessor>& event_processor)
    : descriptor_{-1},
      event_processor_{event_processor} {
  descriptor_ = socket(AF_INET, SOCK_STREAM, 0);

  if (descriptor_ == -1) {
    throw std::runtime_error{"socket creation failed..."};
  }

  int flags = fcntl(descriptor_, F_GETFL, 0);
  fcntl(descriptor_, F_SETFL, flags | O_NONBLOCK);

  event_processor_->RegisterEvent(
    descriptor_,
    EVFILT_READ | EVFILT_WRITE,
    [this](int id) { OnRead(id); },
    [this](int id) { OnWrite(id); });
}

bool TcpSocket::IsValid() const noexcept { return descriptor_ != -1; }

int TcpSocket::NativeHandle() const noexcept { return descriptor_; }

bool TcpSocket::Bind(const std::string& host, unsigned short port) noexcept {
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(inet_addr(host.data()));
  address.sin_port = htons(port);
  return Bind(address);
}

bool TcpSocket::Bind(const sockaddr_in& address) noexcept {
  if (!IsValid()) {
    std::cerr << "socket is in invalid state, recreate TcpSocket object\n";
    return false;
  }

  int result = bind(descriptor_, (sockaddr*) &address, sizeof(address));

  if (result != 0) {
    std::cerr << "socket bind failed...\n";
    Close();
  }

  return true;
}

bool TcpSocket::Listen(const std::function<void(const TcpSocket&)> on_accept) noexcept {
  if (!IsValid()) {
    std::cerr << "socket is in invalid state, recreate TcpSocket object\n";
    return false;
  }

  auto result = listen(descriptor_, SOMAXCONN);

  if (result) {
    std::cerr << errno << ": " << strerror(errno) << std::endl;
  }

  return true;
}

void TcpSocket::Close() noexcept {
  close(descriptor_);
  descriptor_ = -1;
}

void TcpSocket::Shutdown() noexcept { shutdown(descriptor_, SHUT_RDWR); }

void TcpSocket::OnRead(int descriptor) noexcept {
  (void)descriptor;
  std::cout << "on read called\n";

  char buf[4096];
  int n = 0;

  while ((n = read(fd, buf, sizeof buf)) > 0) {
    printf("read %d bytes\n", n);
    int r = write(fd, buf, n);
    exit_if(r <= 0, "write error");
  }
}

void TcpSocket::OnWrite(int descriptor) noexcept {
  (void)descriptor;
  std::cout << "on write called\n";
}