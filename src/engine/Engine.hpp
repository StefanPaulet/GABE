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

  explicit Engine(std::string const& rootFolder, std::string const& csgoRootPath) :
      _windowController {rootFolder + "scripts/find_csXwindow.sh", _synchronizer},
      _positionReader {_state, csgoRootPath + "game/csgo"} {
    buildTrees(rootFolder);
  }

  ~Engine() {
    _windowController.stop();
    _positionReader.stop();
    _windowControllerThread.join();
    _positionReaderThread.join();
    log("Stopped processing commands", OpState::SUCCESS);
  }

  auto run() -> int {
    log("Started processing commands", OpState::SUCCESS);
    _windowControllerThread = _windowController.run();
    _positionReaderThread = _positionReader.run();

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
    buildMovementTree();
  }

  auto buildImageCapturingTree() -> void {
    _trees.push_back(std::make_unique<ImageCapturingTree>(_state, _synchronizer));
  }

  auto buildShootingTree(std::string const& objectDetectionRootPath) -> void {
    _trees.push_back(std::make_unique<ShootingTree>(_state, objectDetectionRootPath));
  }

  auto buildMovementTree() -> void { _trees.push_back(std::make_unique<MovementTree>(_state)); }

  Synchronizer _synchronizer {};
  WindowController _windowController;
  std::jthread _windowControllerThread;
  GameState _state {};
  utils::PositionReader _positionReader;
  std::jthread _positionReaderThread;
  std::vector<std::unique_ptr<DecisionTree>> _trees {};
};
} // namespace gabe