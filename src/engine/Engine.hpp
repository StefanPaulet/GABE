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

  explicit Engine(std::string const& controllerScript) : _windowController {controllerScript, _synchronizer} {}

  ~Engine() {
    _windowController.stop();
    _windowControllerThread.join();
    log("Stopped processing commands", OpState::SUCCESS);
  }

  auto run() -> int {
    log("Started processing commands", OpState::SUCCESS);
    _windowControllerThread = _windowController.run();
    usleep(5000);
    return 0;
  }

private:
  Synchronizer _synchronizer {};
  WindowController _windowController;
  GameState _state {};
  std::jthread _windowControllerThread;
};
} // namespace gabe