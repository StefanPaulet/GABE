//
// Created by stefan on 4/26/24.
//
#include <exception>
#include <string>
#include <utility>

namespace gabe::exceptions {
class DecisionAdditionException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override { return "Weight of decision would go above the limit"; }
};
} // namespace gabe::exceptions
