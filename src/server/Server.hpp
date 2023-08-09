//
// Created by stefan on 8/9/23.
//

#pragma once

#include <exceptions/Exceptions.hpp>
#include <list>
#include <netinet/in.h>
#include <thread/Thread.hpp>
#include <unistd.h>

namespace gabe::server {
class Server {

public:
  Server() noexcept(false);
  [[nodiscard]] auto getClient() const -> int;
  auto createThread(int fd) noexcept(false) -> void;

private:
  inline auto initialize() noexcept(false) -> void;
  static auto threadMain(void* pParam) -> void*;

  static constexpr int PORT = 3000;
  sockaddr_in _properties {};
  int _socketFd {};
  std::list<Thread> _threadList {};
};
} // namespace gabe::server
