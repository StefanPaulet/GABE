//
// Created by stefan on 9/23/23.
//

#pragma once

#include <cstring>
#include <httpMessage/HttpMessage.hpp>
#include <unistd.h>

namespace gabe::server {
class Socket {

public:
  explicit Socket(int const& fd) noexcept;
  ~Socket();
  auto read() -> HttpMessage;
  void write(HttpMessage const& httpMessage);

private:
  static constexpr unsigned long MSG_SIZE = 32768;
  int _fd {};
  char* _pBuf = new char[MSG_SIZE];
};
} // namespace gabe::server
