//
// Created by stefan on 3/28/24.
//

#pragma once
#include <string>

namespace gabe {
enum class OpState { SUCCESS, FAILURE, INFO };

auto log(std::string const& message, OpState opState) -> void {
  std::string formatString {};
  switch (opState) {
    using enum gabe::OpState;
    case SUCCESS: {
      formatString = "\033[1;32m";
      break;
    }
    case FAILURE: {
      formatString = "\033[1;31m";
      break;
    }
    case INFO: {
      formatString = "\033[1;33m";
      break;
    }
  }

  printf("%s %s\n", formatString.c_str(), message.c_str());
}

} // namespace gabe