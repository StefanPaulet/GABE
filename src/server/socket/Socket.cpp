//
// Created by stefan on 9/23/23.
//

#include "Socket.hpp"

namespace {
using namespace gabe::server;
using namespace gabe::server::exception;
} // namespace

Socket::Socket(int const& fd) noexcept : _fd {fd} {}

Socket::~Socket() { delete[] _pBuf; }

auto Socket::read() const noexcept(false) -> HttpMessage {
  if (auto bytesRead = ::read(_fd, _pBuf, MSG_SIZE); 0 >= bytesRead) {
    throw ConnectionTimeoutException();
  }

  auto pTopLine = strchr(_pBuf, '\r');
  if (!pTopLine) {
    return HttpMessage {"\n", HttpMessage::HeaderType(), _pBuf};
  }
  *pTopLine = '\0';

  auto pHeadersFinish = strstr(pTopLine + 2, "\r\n\r\n");
  *(pHeadersFinish + 1) = '\0';

  HttpMessage::HeaderType headerMap {};
  for (auto pHeaderStart = pTopLine + 2, pHeaderEnd = strchr(pHeaderStart, '\r'); pHeaderStart < pHeadersFinish;
       pHeaderStart = pHeaderEnd + 2, pHeaderEnd = strchr(pHeaderStart, '\r')) {
    *pHeaderEnd = '\0';
    auto pHeaderTitle = strchr(pHeaderStart, ':');
    *pHeaderTitle = '\0';
    headerMap.try_emplace(pHeaderStart, pHeaderTitle + 1);
  }

  auto pBody = (*(pHeadersFinish + 4) == '\0') ? "" : (pHeadersFinish + 4);

  return HttpMessage {_pBuf, headerMap, pBody};
}

auto Socket::write(gabe::server::HttpMessage const& httpMessage) const noexcept(false) -> void {
  auto const response_str = httpMessage.to_string();
  auto bytesWritten = ::write(_fd, response_str.c_str(), response_str.size());
  if (0 >= bytesWritten) {
    throw ConnectionTimeoutException();
  }
}