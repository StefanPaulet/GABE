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

  explicit Engine(std::string const& rootFolder) :
      _windowController {rootFolder + "scripts/find_csXwindow.sh", _synchronizer} {
    buildTrees(rootFolder);
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
  auto buildTrees(std::string const& rootFolder) -> void {
    buildImageCapturingTree();
    //    buildShootingTree(rootFolder + "scripts/objectDetection");
    buildMovementTree(rootFolder + "data/templateDigits");
  }

  auto buildImageCapturingTree() -> void {
    _trees.push_back(std::make_unique<ImageCapturingTree>(_state, _synchronizer));
  }

  auto buildShootingTree(std::string const& objectDetectionRootPath) -> void {
    _trees.push_back(std::make_unique<ShootingTree>(_state, objectDetectionRootPath));
  }

  auto buildMovementTree(std::string const& templatePositionDigits) -> void {
    _trees.push_back(std::make_unique<MovementTree>(_state, templatePositionDigits));
  }

  Synchronizer _synchronizer {};
  WindowController _windowController;
  GameState _state {};
  std::jthread _windowControllerThread;
  std::vector<std::unique_ptr<DecisionTree>> _trees {};
};
} // namespace gabe