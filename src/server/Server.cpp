//
// Created by stefan on 8/9/23.
//

#include "Server.hpp"
#include <iostream>

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

auto Server::getClient() -> void {
  sockaddr_in client_addr {};
  socklen_t client_addr_size = sizeof(client_addr);
  int fd;

  if (-1 == (fd = accept(_socketFd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_size))) {
    throw exception::AcceptException();
  }

  createThread(fd);
}

auto Server::createThread(int fd) -> void {
  auto pFd = new int;
  *pFd = fd;
  _threadList.emplace_back(&Server::threadMain, pFd);
}


auto Server::threadMain(void* pParam) -> void* {
  int fd = *static_cast<int*>(pParam);
  delete pParam;
  char buffer[30000] = {0};
  std::string response = "Hello there";
  while (true) {
    if (0 >= read(fd, buffer, sizeof(buffer))) {
      break;
    }
    fflush(stdout);

    HttpMessage message;
    message.setHeader("Content-Type", "text/plain");
    message.setHeader("Content-Length", response.size());
    message.setBody(response);
    sendMessage(fd, message);
  }

  return nullptr;
}

auto Server::sendMessage(int fd, HttpMessage const& message) -> void {
  auto buffer = message.getHeaders() + "\n" + message.getBody();
  write(fd, buffer.c_str(), buffer.size());
}