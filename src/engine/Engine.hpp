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

  explicit Engine(std::string const& controllerScript) :
      _windowController {controllerScript + "/find_csXwindow.sh", _synchronizer} {
    buildTrees(controllerScript);
  }

  ~Engine() {
    _windowController.stop();
    _windowControllerThread.join();
    log("Stopped processing commands", OpState::SUCCESS);
  }

  auto run() -> int {
    log("Started processing commands", OpState::SUCCESS);
    _windowControllerThread = _windowController.run();

    while (true) {
      for (auto const& e : _trees) {
        _windowController.addEvent(e->evaluate());
        e->postEvaluation();
      }
    }
    usleep(5000);
    return 0;
  }

private:
  auto buildTrees(std::string const& scriptsRootPath) -> void {
    buildImageCapturingTree();
    buildShootingTree(scriptsRootPath + "/objectDetection");
  }

  auto buildImageCapturingTree() -> void {
    _trees.push_back(std::make_unique<ImageCapturingTree>(_state, _synchronizer));
  }

  auto buildShootingTree(std::string const& objectDetectionRootPath) -> void {
    _trees.push_back(std::make_unique<ShootingTree>(_state, objectDetectionRootPath));
  }

  Synchronizer _synchronizer {};
  WindowController _windowController;
  GameState _state {};
  std::jthread _windowControllerThread;
  std::vector<std::unique_ptr<DecisionTree>> _trees {};
};
} // namespace gabe