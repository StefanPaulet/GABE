//
// Created by stefan on 4/2/24.
//

#pragma once
#include "DecisionTree.hpp"
#include "GameState.hpp"
#include "windowController/WindowController.hpp"

namespace gabe {
class Engine {
public:
  Engine() = delete;
  Engine(Engine const&) = delete;
  Engine(Engine&&) noexcept = delete;

  explicit Engine(WindowController& windowController) : _windowController {windowController} {}

  ~Engine() {
    _windowController.stop();
    _windowControllerThread.join();
    log("Stopped processing commands", OpState::SUCCESS);
  }

  auto run() -> int {
    log("Started processing commands", OpState::SUCCESS);
    _windowControllerThread = _windowController.run();
    _windowController.add_event(std::make_unique<ScreenshotEvent>());
    usleep(5000);
    return 0;
  }

private:
  WindowController& _windowController;
  GameState _state {};
  std::jthread _windowControllerThread;
};
} // namespace gabe