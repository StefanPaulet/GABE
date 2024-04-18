//
// Created by stefan on 3/28/24.
//

#pragma once

#include "Exceptions.hpp"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cstdio>
#include <event/Event.hpp>
#include <queue>
#include <thread>
#include <utils/logger/Logger.hpp>

namespace gabe {
class WindowController {
public:
  WindowController() = delete;
  WindowController(WindowController const&) = delete;
  WindowController(WindowController&&) noexcept = delete;

  explicit WindowController(std::string const& pathToWindowFinder) noexcept(false) {
    _display = XOpenDisplay(nullptr);
    if (_display == nullptr) {
      throw window::DisplayOpeningException {};
    }

    log("Searching for Counter-Strike 2 XWindow", OpState::INFO);
    if (!findWindow(pathToWindowFinder)) {
      log("Terminating program", OpState::FAILURE);
    }
  }

  [[nodiscard]] auto run() -> std::jthread {
    auto eventLoop = [this] {
      while (!stopped) {
        try {
          checkWindowState();
        } catch (window::DisplayOpeningException const& e) {
          log("Error: " + std::string(e.what()), OpState::FAILURE);
        }
        if (_windowState.focused == false || _eventQueue.empty()) {
          continue;
        }
        auto event = std::move(_eventQueue.front());
        _eventQueue.pop();
        event.get()->solve(_display, _window);
      }
      usleep(500000);
      log("Finished running the event loop", OpState::SUCCESS);
    };
    return std::jthread {eventLoop};
  }

  auto add_event(std::unique_ptr<Event>&& ev) { _eventQueue.push(std::move(ev)); }

  auto stop() { stopped = true; }

private:
  struct WindowState {
    bool focused {true};
  };
  bool volatile stopped {false};
  WindowState _windowState {};
  Window _window {};
  Display* _display;
  std::queue<std::unique_ptr<Event>> _eventQueue {};

  auto checkWindowState() noexcept(false) -> void {
    Window focusedWindow;
    int revertTo;
    XGetInputFocus(_display, &focusedWindow, &revertTo);
    bool windowFocused = (focusedWindow == _window) || checkWindowAncestry(focusedWindow);

    if (_windowState.focused != windowFocused) {
      _windowState.focused = windowFocused;
      if (!_windowState.focused) {
        log("Target window lost focus, disabling input", OpState::INFO);
      } else {
        log("Target window gained focus, enabling input", OpState::INFO);
      }
    }
  }

  auto checkWindowAncestry(Window targetWindow) -> bool {
    Window root;
    Window parent;
    Window* children;
    unsigned int num_children;

    while (true) {
      if (!XQueryTree(_display, targetWindow, &root, &parent, &children, &num_children)) {
        throw window::TreeQueryException {};
      }
      if (children != nullptr) {
        XFree(children);
      }
      if (parent == root || parent == None) {
        break;
      }
      if (parent == _window) {
        return true;
      }
      targetWindow = parent;
    }
    return false;
  }

  auto findWindow(std::string const& cmd) noexcept(false) -> bool {
    auto exec = [](char const* cmd) {
      std::array<char, 128> buffer {};
      std::string result;
      std::unique_ptr<FILE, decltype(&pclose)> pipe {popen(cmd, "r"), &pclose};
      if (!pipe) {
        throw window::PipeOpenException {};
      }
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += std::string {buffer.data()};
      }
      return result;
    };

    try {
      std::string output = exec(cmd.c_str());
      _window = std::stoi(output, nullptr, 16);
      log(std::format("Found Counter-Strike 2 window id: {}", _window), OpState::SUCCESS);
      return true;
    } catch (window::PipeOpenException const& e) {
      log("Error: " + std::string {e.what()}, OpState::FAILURE);
    }
    return false;
  }
};
} // namespace gabe