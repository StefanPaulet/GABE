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
    _headers {headers}, _body {body + "\n"} {
  std::cout << "Default constructor\n";
}

HttpMessage::HttpMessage(std::string&& topLine, gabe::server::HttpMessage::HeaderType&& headers, std::string&& body) :
    _topLine {std::move(topLine)}, _headers {std::move(headers)}, _body {std::move(body)} {
  std::cout << "Move constructor\n";
}

constexpr auto HttpMessage::setBody(std::string const& body) -> void { _body = body + "\n"; }

constexpr auto HttpMessage::setTopLine(std::string const& topLine) -> void { _topLine = topLine + "\n"; }

constexpr auto HttpMessage::getHeaders() const -> HeaderType const& { return _headers; }

constexpr auto HttpMessage::getBody() const -> std::string const& { return _body; }

constexpr auto HttpMessage::getTopLine() const -> std::string const& { return _topLine; }

auto gabe::server::operator<<(std::ostream& out, HttpMessage const& message) -> std::ostream& {
  out << message._topLine;
}