//
// Created by stefan on 8/21/23.
//

#pragma once

#include <iostream>
#include <map>
#include <string>

namespace gabe::server {

template <typename T>
concept Stringable = requires(T val) {
  { std::to_string(val) } -> std::convertible_to<std::string>;
};

class HttpMessage {
public:
  using HeaderType = std::map<std::string, std::string, std::less<>>;

  HttpMessage(std::string const& topLine, HeaderType const& headers, std::string const& body) :
      _topLine {topLine + "\n"}, _headers {headers}, _body {body + "\n"} {}

  HttpMessage(std::string&& topLine, HeaderType&& headers, std::string&& body) :
      _topLine {std::move(topLine)}, _headers {std::move(headers)}, _body {std::move(body)} {}

  template <Stringable HeaderContentType> auto setHeader(std::string const& name, HeaderContentType const& content)
      -> void {
    _headers.insert(name, std::to_string(content));
  }

  template <typename HeaderContentType> auto setHeader(std::string const& name, HeaderContentType const& content)
      -> void {
    _headers.emplace(name, content);
  }

  [[nodiscard]] constexpr auto getBody() const -> std::string const& { return _body; }

  [[nodiscard]] auto to_string() const -> std::string {
    std::string res = _topLine;
    for (auto& [header, value] : _headers) {
      res += header;
      res += ":";
      res += value;
      res += "\n";
    }
    return res + _body;
  }

  friend auto operator<<(std::ostream& out, HttpMessage const& message) -> std::ostream& {
    out << "Method:" << message._topLine;
    for (auto& [header, value] : message._headers) {
      out << header << ":" << value << '\n';
    }
    out << "Body:" << message._body;
    return out;
  }

private:
  std::string _topLine {};
  HeaderType _headers {};
  std::string _body {};
};
} // namespace gabe::server
