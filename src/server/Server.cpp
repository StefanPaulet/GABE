//
// Created by stefan on 8/9/23.
//

#include "Server.hpp"


namespace {
using namespace gabe::server;
} // namespace

Server::Server() {
  _properties.sin_family = AF_INET;
  _properties.sin_addr.s_addr = htonl(INADDR_ANY);
  _properties.sin_port = htons(PORT);
  initialize();
}

auto Server::initialize() -> void {

  if (-1 == (_socketFd = socket(AF_INET, SOCK_STREAM, 0))) {
    throw exception::SocketCreationException();
  }

  if (int optionToggle = 1; -1 == setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &optionToggle, sizeof(int))) {
    throw exception::SocketOptionException();
  }

  if (-1 == bind(_socketFd, reinterpret_cast<sockaddr*>(&_properties), sizeof(sockaddr))) {
    throw exception::SocketBindException();
  }

  if (-1 == listen(_socketFd, 1)) {
    throw exception::ListenException();
  }
}

auto Server::getClient() const -> int {
  sockaddr_in client_addr {};
  socklen_t client_addr_size = sizeof(client_addr);
  int fd;

  if (-1 == (fd = accept(_socketFd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_size))) {
    throw exception::AcceptException();
  }

  return fd;
}

auto Server::createThread(int fd) -> void {
  auto pFd = new int;
  *pFd = fd;
  _threadList.emplace_back(&Server::threadMain, pFd);
}


auto Server::threadMain(void* pParam) -> void* {
  int fd = *static_cast<int*>(pParam);
  char buffer[1024];
  while (true) {
    if (0 >= read(fd, buffer, 1024)) {
      break;
    }
    write(1, buffer, 1024);
  }

  return nullptr;
}
