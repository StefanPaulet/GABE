//
// Created by stefan on 3/29/24.
//

#pragma once
#include <exception>

namespace gabe::window {

class DisplayOpeningException : public std::exception {
public:
  [[nodiscard]] char const* what() const noexcept override { return "Could not open display"; }
};

class PipeOpenException : public std::exception {
public:
  [[nodiscard]] char const* what() const noexcept override { return "Could not open pipe to window-finding script"; }
};

class TreeQueryException : public std::exception {
public:
  [[nodiscard]] char const* what() const noexcept override { return "Could not use X tree query"; }
};


} // namespace gabe::window