//
// Created by stefan on 4/2/24.
//

#pragma once
#include "DecisionTree.hpp"
#include "GameState.hpp"
#include "Path.hpp"
#include "gameStateIntegrator/Integrator.hpp"
#include "positionReader/PositionReader.hpp"
#include "windowController/WindowController.hpp"

namespace gabe {
class Engine {
public:
  Engine() = delete;
  Engine(Engine const&) = delete;
  Engine(Engine&&) noexcept = delete;

  explicit Engine(std::string const& rootFolder, std::string const& csgoRootPath) :
      _windowController {rootFolder + "scripts/find_csXwindow.sh", _synchronizer},
      _positionReader {_state, csgoRootPath + "game/csgo"}, _integrator {_state} {
    buildTrees(rootFolder);
  }

  ~Engine() {
    _windowController.stop();
    _positionReader.stop();
    _integrator.stop();
    log("Stopped processing commands", OpState::SUCCESS);
  }

  auto run() -> int {
    log("Started processing commands", OpState::SUCCESS);
    _windowController.run();
    _positionReader.run();
    _integrator.run();

    setupCommands();

    while (true) {
      if (_state.round().stage() == Round::Stage::OVER) {
        continue;
      }
      for (auto const& e : _trees) {
        _windowController.addEvent(e->evaluate());
        e->postEvaluation();
      }
      _synchronizer.requestSynchronization();
      usleep(6000);
    }
    return 0;
  }

private:
  auto buildTrees(std::string const& rootFolder) -> void {
    buildImageCapturingTree();
    buildShootingTree(rootFolder + "scripts/objectDetection");
    buildPositionGettingTree();
    buildTargetChoosingTree();
    buildAimingTree();
    buildMovementTree();
    buildWeaponsChoosingTree();
    buildSituationalTrees();
  }

#ifndef NDEBUG
  auto setupCommands() -> void {
    _windowController.addEvent(std::make_unique<CommandEvent>("sv_cheats true"));
    _windowController.addEvent(std::make_unique<CommandEvent>("bind p getpos"));
    _windowController.addEvent(std::make_unique<CommandEvent>("mp_autoteambalance false"));
    _windowController.addEvent(std::make_unique<CommandEvent>("mp_roundtime 600"));
    _windowController.addEvent(std::make_unique<CommandEvent>("mp_roundtime_defuse 600"));
    _windowController.addEvent(std::make_unique<CommandEvent>("mp_limitteams 5"));
    _windowController.addEvent(std::make_unique<CommandEvent>("sv_infinite_ammo 2"));
    //_windowController.addEvent(std::make_unique<CommandEvent>("bot_kick"));
    //_windowController.addEvent(std::make_unique<CommandEvent>("bot_add ct"));
    _windowController.addEvent(std::make_unique<CommandEvent>("bot_stop 0"));
  }

#else
  auto setupCommands() -> void {
    _windowController.addEvent(std::make_unique<CommandEvent>("sv_cheats true"));
    _windowController.addEvent(std::make_unique<CommandEvent>("bind p getpos"));
  }
#endif

  auto buildImageCapturingTree() -> void {
    _trees.push_back(std::make_unique<ImageCapturingTree>(_state, _synchronizer));
  }

  auto buildShootingTree(std::string const& objectDetectionRootPath) -> void {
    auto shootingTreeRoot = std::make_unique<EnemyDetectionTree>(_state, objectDetectionRootPath);
    shootingTreeRoot->addDecision(0.8f, std::make_unique<SlowShootingTree>(_state));
    shootingTreeRoot->addDecision(0.2f, std::make_unique<SprayShootingTree>(_state));
    _trees.push_back(std::move(shootingTreeRoot));
  }

  auto buildMovementTree() -> void { _trees.push_back(std::make_unique<MovementTree>(_state)); }

  auto buildPositionGettingTree() -> void {
    _trees.push_back(std::make_unique<PositionGettingTree>(_state, _positionReader.synchronizer));
  }

  auto buildAimingTree() -> void {
    auto aimingRootTree = std::make_unique<AimingTree>(_state);
    aimingRootTree->addDecision(0.55f, std::make_unique<MovementOrientedRotationTree>(_state));
    aimingRootTree->addDecision(0.4f, std::make_unique<AimingOrientedRotationTree>(_state));
    aimingRootTree->addDecision(0.05f, std::make_unique<BackCheckingRotationTree>(_state));
    _trees.push_back(std::move(aimingRootTree));
  }

  auto buildTargetChoosingTree() -> void {
    auto destinationTree = std::make_unique<DestinationChoosingTree>(_state);
    auto pathChoosingTree = std::make_unique<PathChoosingTree<ShortestPathPolicy>>(_state);
    pathChoosingTree->addDecision(1.0f, std::make_unique<LocationChoosingTree<DirectMovementPolicy>>(_state));
    destinationTree->addDecision(1.0f, std::move(pathChoosingTree));
    _trees.push_back(std::move(destinationTree));
  }

  auto buildWeaponsChoosingTree() -> void { _trees.push_back(std::make_unique<WeaponChoosingTree>(_state)); }

  auto buildSituationalTrees() -> void {
    _trees.push_back(std::make_unique<BombPlantingTree>(_state));
    _trees.push_back(std::make_unique<BuyingTree>(_state));
  }

  Synchronizer _synchronizer {};
  WindowController _windowController;
  GameState _state {};
  PositionReader _positionReader;
  std::vector<std::unique_ptr<DecisionTree>> _trees {};
  Integrator _integrator;
};
} // namespace gabe
