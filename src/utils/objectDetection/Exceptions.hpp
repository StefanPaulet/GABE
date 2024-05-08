//
// Created by stefan on 4/26/24.
//
#include <exception>
#include <string>
#include <utility>

namespace gabe::utils::exceptions {
class PipeCreationException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Could not create pipe"; }
};
class ChildCreationException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Could not create child process"; }
};
class ChildCommunicationException : public std::exception {
public:
  ChildCommunicationException(std::string const& expected, std::string const& received) :
      _msg {"Child communication failure; expected:" + expected + " but received:" + received} {}

  [[nodiscard]] char const* what() const noexcept override { return _msg.c_str(); }

private:
  std::string _msg;
};
} // namespace gabe::utils::exceptions
