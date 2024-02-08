//
// Created by stefan on 8/21/23.
//

#include "HttpMessage.hpp"

namespace {
using namespace gabe::server;
} // namespace

HttpMessage::HttpMessage(std::string const& topLine, gabe::server::HttpMessage::HeaderType const& headers,
                         std::string const& body) :
    _topLine {topLine + "\n"},
    _headers {headers}, _body {body + "\n"} {}

HttpMessage::HttpMessage(std::string&& topLine, gabe::server::HttpMessage::HeaderType&& headers, std::string&& body) :
    _topLine {std::move(topLine)}, _headers {std::move(headers)}, _body {std::move(body)} {}

constexpr auto HttpMessage::setBody(std::string const& body) -> void { _body = body + "\n"; }

constexpr auto HttpMessage::setTopLine(std::string const& topLine) -> void { _topLine = topLine + "\n"; }

constexpr auto HttpMessage::getHeaders() const -> HeaderType const& { return _headers; }

constexpr auto HttpMessage::getBody() const -> std::string const& { return _body; }

constexpr auto HttpMessage::getTopLine() const -> std::string const& { return _topLine; }

auto HttpMessage::to_string() const -> std::string {
  std::string res = _topLine;
  for (auto& [header, value] : _headers) {
    res += header;
    res += ":";
    res += value;
    res += "\n";
  }
  return res + _body;
}

auto gabe::server::operator<<(std::ostream& out, HttpMessage const& message) -> std::ostream& {
  out << "Method:" << message._topLine;
  for (auto& [header, value] : message._headers) {
    out << header << ":" << value << '\n';
  }
  out << "Body:" << message._body;
  return out;
}
