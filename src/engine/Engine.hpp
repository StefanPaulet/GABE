//
// Created by stefan on 4/2/24.
//

#pragma once
#include "DecisionTree.hpp"
#include "GameState.hpp"
#include "Path.hpp"
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

    setupCommands();

    while (true) {
      for (auto const& e : _trees) {
        _windowController.addEvent(e->evaluate());
        e->postEvaluation();
      }
      _windowController.addEvent(std::make_unique<KeyPressEvent>('p', 100));
    }
    usleep(5000);
    return 0;
  }

private:
  auto buildTrees(std::string const& rootFolder) -> void {
    buildImageCapturingTree();
    buildShootingTree(rootFolder + "scripts/objectDetection");
    buildMovementTree();
    buildTargetChoosingTree();
  }

  auto setupCommands() -> void {
    _windowController.addEvent(std::make_unique<CommandEvent>("sv_cheats true"));
    _windowController.addEvent(std::make_unique<CommandEvent>("keybind p getpos"));
  }

  auto buildImageCapturingTree() -> void {
    _trees.push_back(std::make_unique<ImageCapturingTree>(_state, _synchronizer));
  }

  auto buildShootingTree(std::string const& objectDetectionRootPath) -> void {
    _trees.push_back(std::make_unique<ShootingTree>(_state, objectDetectionRootPath));
  }

  auto buildMovementTree() -> void {
    auto movementTree = std::make_unique<PositionGettingTree>(_state);
    movementTree->addDecision(1, std::make_unique<MovementTree>(_state));
    _trees.push_back(std::move(movementTree));
  }

  auto buildTargetChoosingTree() -> void {
    auto destinationTree = std::make_unique<DestinationChoosingTree>(_state);
    destinationTree->addDecision(1, std::make_unique<PathChoosingTree<path::ShortestPathPolicy>>(_state));
    _trees.push_back(std::move(destinationTree));
  }

  Synchronizer _synchronizer {};
  WindowController _windowController;
  std::jthread _windowControllerThread;
  GameState _state {};
  utils::PositionReader _positionReader;
  std::jthread _positionReaderThread;
  std::vector<std::unique_ptr<DecisionTree>> _trees {};
};
} // namespace gabe
