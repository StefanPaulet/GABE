//
// Created by stefan on 5/15/24.
//

#pragma once

namespace gabe::utils::exceptions {
class InvalidCSRootPathException : public std::exception {
  [[nodiscard]] char const* what() const noexcept override {
    return "The given filepath for the CS2 root does not exist";
  }
};
} // namespace gabe::utils::exceptions
