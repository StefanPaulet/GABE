//
// Created by stefan on 8/9/23.
//

#pragma once

#include <exceptions/Exceptions.hpp>
#include <httpMessage/HttpMessage.hpp>
#include <list>
#include <netinet/in.h>
#include <thread/Thread.hpp>
#include <unistd.h>

namespace gabe::server {
class Server {
public:
  Server() noexcept(false);
  auto getClient() -> void;
  static auto sendMessage(int fd, HttpMessage const& message) -> void;

private:
  auto createThread(int fd) noexcept(false) -> void;
  inline auto initialize() noexcept(false) -> void;
  static auto threadMain(void* pParam) -> void*;

  static constexpr int PORT = 3000;
  sockaddr_in _properties {};
  int _socketFd {};
  std::list<Thread> _threadList {};
};
} // namespace gabe::server
