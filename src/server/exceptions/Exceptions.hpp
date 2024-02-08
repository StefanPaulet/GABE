//
// Created by stefan on 8/9/23.
//

#pragma once

#include <exception>

namespace gabe::server::exception {
class SocketCreationException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Could not create socket"; }
};

class SocketOptionException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Could not set socket options"; }
};

class SocketBindException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Could not bind socket"; }
};

class ListenException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Could not listen on socket"; }
};

class AcceptException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Could not accept client"; }
};

class ThreadCreationException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Could not create thread"; }
};

class ConnectionTimeoutException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Connection from client dropped"; }
};
} // namespace gabe::server::exception
