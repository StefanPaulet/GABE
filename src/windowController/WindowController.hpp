//
// Created by stefan on 3/28/24.
//

#pragma once

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cstdio>
#include <event/Event.hpp>
#include <utils/logger/Logger.hpp>

namespace gabe {
class WindowController {
private:
public:
  WindowController() {
    log("Searching for Counter-Strike 2 XWindow", OpState::INFO);
    if (!findWindow()) {
      log("Terminating program", OpState::FAILURE);
    }
  }
  WindowController(WindowController const&) = delete;
  WindowController(WindowController&&) noexcept = delete;

private:
  Window _window {};

  auto findWindow() -> bool {
    auto exec = [](char const* cmd) {
      std::array<char, 128> buffer {};
      std::string result;
      std::unique_ptr<FILE, decltype(&pclose)> pipe {popen(cmd, "r"), &pclose};
      if (!pipe) {
        throw std::runtime_error("popen() failed!");
      }
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += std::string {buffer.data()};
      }
      return result;
    };

    try {
      std::string output = exec("../scripts/find_csXwindow.sh");
      _window = std::stoi(output, nullptr, 16);
      log("Found Counter-Strike 2 window id: " + std::to_string(_window), OpState::SUCCESS);
      return true;
    } catch (std::exception const& e) {
      log("Error " + std::string {e.what()}, OpState::FAILURE);
    }
    return false;
  }
};
} // namespace gabe