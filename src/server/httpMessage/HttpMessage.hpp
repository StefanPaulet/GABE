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

  HttpMessage(std::string const& topLine, HeaderType const& headers, std::string const& body);
  HttpMessage(std::string&& topLine, HeaderType&& headers, std::string&& body);

  template <Stringable HeaderContentType> auto setHeader(std::string const& name, HeaderContentType const& content)
      -> void {
    _headers.insert(name, std::to_string(content));
  }

  template <typename HeaderContentType> auto setHeader(std::string const& name, HeaderContentType const& content)
      -> void {
    _headers.emplace(name, content);
  }
  constexpr auto setBody(std::string const& body) -> void;

  constexpr auto setTopLine(std::string const& topLine) -> void;

  [[nodiscard]] constexpr auto getHeaders() const -> HeaderType const&;
  [[nodiscard]] constexpr auto getBody() const -> std::string const&;
  [[nodiscard]] constexpr auto getTopLine() const -> std::string const&;

  friend auto operator<<(std::ostream& out, HttpMessage const& message) -> std::ostream&;

private:
  std::string _topLine {};
  HeaderType _headers {};
  std::string _body {};
};
} // namespace gabe::server
