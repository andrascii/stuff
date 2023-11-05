#pragma once

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ievent_processor.h"

//
// if some function fails you need to destroy the socket and create a new one
//
class TcpSocket {
 public:
  TcpSocket(const std::shared_ptr<IEventProcessor>& event_processor);

  bool IsValid() const noexcept;

  int NativeHandle() const noexcept;

  bool Bind(const std::string& host, unsigned short port) noexcept;
  bool Bind(const sockaddr_in& address) noexcept;

  bool Listen(const std::function<void(const TcpSocket&)> on_accept) noexcept;

  void Close() noexcept;
  void Shutdown() noexcept;

 private:
  void OnRead(int descriptor) noexcept;
  void OnWrite(int descriptor) noexcept;

 private:
  int descriptor_;
  std::shared_ptr<IEventProcessor> event_processor_;
};