//
// Created by stefan on 8/9/23.
//

#include "Server.hpp"
#include <CDS/util/JSON>
#include <fstream>
#include <mutex>

namespace {
using namespace gabe::server;
using namespace gabe::server::exception;
} // namespace

Server::Server() {
  _properties.sin_family = AF_INET;
  _properties.sin_addr.s_addr = htonl(INADDR_ANY);
  _properties.sin_port = htons(PORT);
  initialize();
}

auto Server::initialize() -> void {

  if (-1 == (_socketFd = socket(AF_INET, SOCK_STREAM, 0))) {
    throw SocketCreationException();
  }

  if (int optionToggle = 1; -1 == setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &optionToggle, sizeof(int))) {
    throw SocketOptionException();
  }

  if (-1 == bind(_socketFd, reinterpret_cast<sockaddr*>(&_properties), sizeof(sockaddr))) {
    throw SocketBindException();
  }

  if (-1 == listen(_socketFd, 1)) {
    throw ListenException();
  }
}

auto Server::getClient() -> void {
  sockaddr_in client_addr {};
  socklen_t client_addr_size = sizeof(client_addr);
  int fd;

  if (-1 == (fd = accept(_socketFd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_size))) {
    throw AcceptException();
  }

  createThread(fd);
}

auto Server::createThread(int fd) -> void {
  auto pFd = new int;
  *pFd = fd;
  _threadList.emplace_back(&Server::threadMain, pFd);
}


auto Server::threadMain(void* pParam) -> void* {
  static std::mutex myMutex {};
  static auto cout = std::ofstream("fisier.out");

  int fd = *static_cast<int*>(pParam);
  delete static_cast<int*>(pParam);
  HttpMessage response {"Http 1.1/ 200 OK", {}, "ok"};
  Socket const socket {fd};

  try {
    HttpMessage request = socket.read();
    auto lg = std::lock_guard {myMutex};
    auto jsonData = cds::json::parseJson(request.getBody());
    cout << jsonData.getJson("player").getString("name") << '\n';
    cout.flush();
    socket.write(response);
  } catch (ConnectionTimeoutException& e) {
    //empty on purpose
  }

  close(fd);

  return nullptr;
}