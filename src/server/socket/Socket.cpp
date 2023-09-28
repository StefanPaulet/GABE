//
// Created by stefan on 9/23/23.
//

#include "Socket.hpp"

namespace {
using namespace gabe::server;
} // namespace

Socket::Socket(int const& fd) noexcept : _fd {fd} {}

Socket::~Socket() { delete[] _pBuf; }

auto Socket::read() -> HttpMessage {
  ::read(_fd, _pBuf, MSG_SIZE);

  auto pTopLine = strchr(_pBuf, '\r');
  *pTopLine = '\0';

  auto pHeadersFinish = strstr(pTopLine + 2, "\r\n\r\n");
  *pHeadersFinish = '\0';

  HttpMessage::HeaderType headerMap {};
  for (auto pHeaderStart = pTopLine + 2, pHeaderEnd = strchr(pHeaderStart, '\r'); pHeaderStart != nullptr;
       pHeaderStart = pHeaderEnd + 2, pHeaderEnd = strchr(pHeaderStart, '\r')) {
    *pHeaderEnd = '\n';
    *(pHeaderEnd + 1) = '\0';
    auto pHeaderTitle = strchr(pHeaderStart, ':');
    *pHeaderTitle = '\0';
    headerMap.try_emplace(pHeaderStart, pHeaderTitle + 1);
  }

  return HttpMessage {pTopLine, headerMap, pHeadersFinish + 2};
}
