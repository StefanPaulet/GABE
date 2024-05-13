//
// Created by stefan on 3/28/24.
//

#pragma once

#include <cassert>
#include <format>
#include <string>

namespace gabe {
enum class OpState { SUCCESS, FAILURE, INFO };

#ifndef NDEBUG
auto log(std::string const& message, OpState opState) -> void {
  auto formatString = [opState]() {
    switch (opState) {
      using enum gabe::OpState;
      case SUCCESS: return "\033[1;32m";
      case FAILURE: return "\033[1;31m";
      case INFO: return "\033[1;33m";
    }
    assert(false && "OpState undefined");
    return "<undefined operation state>";
  };

  printf("%s %s\n", formatString(), message.c_str());
}
#else
template <typename... Args> auto log(Args&&...) {}
#endif

} // namespace gabe