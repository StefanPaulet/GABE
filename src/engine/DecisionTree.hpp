//
// Created by stefan on 5/9/24.
//

#pragma once

#include "Exceptions.hpp"
#include "GameState.hpp"
#include "Movement.hpp"
#include "Path.hpp"
#include "event/Event.hpp"
#include "multithreaded/synchronizer/Synchronizer.hpp"
#include "utils/objectDetection/ObjectDetectionController.hpp"
#include <memory>
#include <numeric>
#include <random>
#include <vector>

namespace gabe {
constexpr Point screenPoint {expectedScreenWidth, expectedScreenHeight};

class DecisionTree;

struct Decision {
  Decision() = default;
  Decision(Decision const& other) = delete;
  Decision(Decision&& other) = default;
  Decision(float weight, std::unique_ptr<DecisionTree>&& decision) : weight(weight), decision(std::move(decision)) {}

  float weight {};
  std::unique_ptr<DecisionTree> decision;
};

Decision emptyDecision;

class DecisionTree {
public:
  DecisionTree() = delete;
  DecisionTree(DecisionTree const&) = delete;
  DecisionTree(DecisionTree&&) noexcept = default;

  explicit DecisionTree(GameState& state) : _state {state} {}
  DecisionTree(std::vector<Decision>&& list, GameState& state) : _state {state} {
    for (auto& e : list) {
      _children.emplace_back(e.weight, std::move(e.decision));
    }
  }
  virtual ~DecisionTree() = default;

  auto addDecision(float weight, std::unique_ptr<DecisionTree>&& decision) noexcept(false) -> void {
    auto currentWeights = std::accumulate(_children.begin(), _children.end(), .0f,
                                          [](float lhs, Decision const& rhs) { return lhs + rhs.weight; });
    if (currentWeights + weight > 1.0f) {
      throw exceptions::DecisionAdditionException();
    }
    _children.emplace_back(weight, std::move(decision));
  }

  virtual auto randomDecision() -> Decision& { return emptyDecision; }
  virtual auto act() -> std::unique_ptr<Event> { return std::make_unique<EmptyEvent>(); }
  virtual auto postEvaluation() -> void {
    //empty on purpose
  }

  virtual auto evaluate() -> std::unique_ptr<Event> {
    if (_children.empty()) {
      return act();
    }

    auto const& child = selectChild();
    return child.decision->evaluate();
  }

protected:
  auto selectChild() -> Decision& {
    std::random_device rd {};
    auto targetValue = std::uniform_real_distribution<float> {}(rd);
    auto chosenValue = .0f;
    for (auto& e : _children) {
      chosenValue += e.weight;
      if (chosenValue >= targetValue) {
        return e;
      }
    }
    return randomDecision();
  }

  GameState& _state;
  std::vector<Decision> _children;
};

template <int iterationDelay> class EnemyDependentTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto evaluate() -> std::unique_ptr<Event> override {
    if (_state.enemy != utils::sentinelBox) {
      _iterationsWaited = iterationDelay;
    }
    if (_iterationsWaited == 0) {
      return DecisionTree::evaluate();
    }
    --_iterationsWaited;
    return std::make_unique<EmptyEvent>();
  }

private:
  int _iterationsWaited {iterationDelay};
};

class ImageCapturingTree : public DecisionTree {
public:
  ImageCapturingTree(GameState& gameState, Synchronizer& synchronizer) :
      DecisionTree {gameState}, _synchronizer {synchronizer} {
    _image = new unsigned char[expectedScreenHeight * expectedScreenWidth * 3];
  }

  ~ImageCapturingTree() override { delete[] _image; }

  auto act() -> std::unique_ptr<Event> override { return std::make_unique<ScreenshotEvent>(_image); }
  auto postEvaluation() -> void override {
    _synchronizer.requestSynchronization();
    _state.set(GameState::Properties::IMAGE, Image {_image});
  }

private:
  Synchronizer& _synchronizer;
  unsigned char* _image {};
};

class EnemyDetectionTree : public DecisionTree {
public:
  EnemyDetectionTree(GameState& gameState, std::string const& scriptPath) :
      DecisionTree {gameState}, _objectDetectionController {scriptPath} {}

  auto evaluate() -> std::unique_ptr<Event> override {
    auto* image = _state.image().data;
    if (auto enemyList = _objectDetectionController.analyzeImage(image); !enemyList.empty()) {
      auto pointChooser = [](utils::BoundingBox const& b1, utils::BoundingBox const& b2) {
        return ((b1.topLeft + b1.bottomRight - screenPoint) / 2).abs()
            < ((b2.topLeft + b2.bottomRight - screenPoint) / 2).abs();
      };
      _state.enemy = *std::ranges::min_element(enemyList, pointChooser);
    } else {
      _state.enemy = utils::sentinelBox;
    }
    return DecisionTree::evaluate();
  }

private:
  utils::ObjectDetectionController _objectDetectionController;
};

class ShootingTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto act() -> std::unique_ptr<Event> override {
    if (_state.enemy == utils::sentinelBox
        || _state.inventory().currentWeaponState() == Inventory::ActiveWeaponState::RELOADING) {
      return std::make_unique<EmptyEvent>();
    }
    return shoot();
  }

  virtual auto shoot() -> std::unique_ptr<Event> = 0;
};

class SlowShootingTree : public ShootingTree {
public:
  using ShootingTree::ShootingTree;

  auto shoot() -> std::unique_ptr<Event> override {
    auto timeNow = std::chrono::system_clock::now();

    if (std::chrono::duration_cast<std::chrono::microseconds>(timeNow - lastShootTime).count() < shootWaitTime) {
      return std::make_unique<EmptyEvent>();
    }

    auto shootingPoint =
        (_state.enemy.topLeft + (_state.enemy.bottomRight - _state.enemy.topLeft) / Point {2, 5} - screenPoint / 2) * 2;
    ShootEvent::AimType aimType;
    if (std::abs(shootingPoint.x) > 50 || std::abs(shootingPoint.y) > 50) {
      aimType = ShootEvent::AimType::FLICK;
    } else {
      aimType = ShootEvent::AimType::TAP;
    }
    lastShootTime = timeNow;
    return std::make_unique<ShootEvent>(shootingPoint, aimType);
  }

private:
  static constexpr auto shootWaitTime = 250000;
  decltype(std::chrono::system_clock::now()) lastShootTime {};
};

class SprayShootingTree : public ShootingTree {
public:
  using ShootingTree::ShootingTree;

  auto shoot() -> std::unique_ptr<Event> override {
    auto shootingPoint = (_state.enemy.topLeft + _state.enemy.bottomRight) - screenPoint;
    int bulletCount {};
    if (_state.inventory().currentWeaponClass() == Inventory::WeaponClass::WC_PRIMARY) {
      bulletCount = 6;
    } else {
      bulletCount = 4;
    }
    bulletCount = std::max(bulletCount, _state.inventory().currentWeapon().ammo);
    return std::make_unique<SprayEvent>(shootingPoint, bulletCount, _state.inventory().currentWeapon().weapon);
  }
};

class PositionGettingTree : public DecisionTree {
public:
  PositionGettingTree(GameState& gameState, Synchronizer& synchronizer) :
      DecisionTree {gameState}, _synchronizer {synchronizer} {}

  auto act() -> std::unique_ptr<Event> override { return std::make_unique<KeyPressEvent>('p', 100); }
  auto postEvaluation() -> void override { _synchronizer.requestSynchronization(); }

private:
  Synchronizer& _synchronizer;
};

class DestinationChoosingTree : public EnemyDependentTree<3> {
public:
  using EnemyDependentTree::EnemyDependentTree;

  auto evaluate() -> std::unique_ptr<Event> override {
    if (_state.round().bombState() == Round::BombState::PLANTED) {
      _state.targetZone.name = Map::ZoneName::TOP_OF_RAMP;
    } else {
      _state.targetZone.name = Map::ZoneName::A_SITE;
    }
    return EnemyDependentTree::evaluate();
  }
};

template <typename PathChoosingPolicy> class PathChoosingTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto evaluate() -> std::unique_ptr<Event> override {
    _policy.getPath(_state.map.findZone(_state.position()).name, _state.targetZone.name);
    _state.currentPath = _policy.path;
    return DecisionTree::evaluate();
  }

private:
  PathChoosingPolicy _policy {_state.map};
};

template <typename MovementPolicy> class LocationChoosingTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto act() -> std::unique_ptr<Event> override {
    auto position = _state.position();
    auto currentZone = _state.map.findZone(position);
    auto nextZone = _state.currentPath.back();
    auto nextPoint = MovementPolicy {_state.map}.getNextPoints(position, currentZone.zone, _state.currentPath).back();
    _state.nextPosition = nextPoint;
    return std::make_unique<EmptyEvent>();
  }
};

class AimingTree : public EnemyDependentTree<5> {
public:
  using EnemyDependentTree::EnemyDependentTree;
};

class RotationTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto act() -> std::unique_ptr<Event> override {
    auto orientation = _state.orientation();
    auto moveAngle = orientation.y - _targetAngle;
    if (std::abs(moveAngle) > 180.0f) {
      moveAngle = 360 - moveAngle * (moveAngle < 0.0f ? -1.0f : 1.0f);
    }
    auto oldXOrientation = orientation.x;
    orientation.y = _targetAngle;
    orientation.x = 0;
    _state.set(GameState::Properties::ORIENTATION, orientation);
    return std::make_unique<RotationEvent>(moveAngle, -oldXOrientation);
  }


protected:
  float _targetAngle {};
};

class MovementOrientedRotationTree : public RotationTree {
public:
  using RotationTree::RotationTree;

  auto act() -> std::unique_ptr<Event> override {
    auto position = _state.position();
    auto targetPosition = _state.nextPosition;
    _targetAngle = Vector(position, targetPosition).getAngle();
    return RotationTree::act();
  }
};

class AimingOrientedRotationTree : public MovementOrientedRotationTree {
public:
  using MovementOrientedRotationTree::MovementOrientedRotationTree;

  auto act() -> std::unique_ptr<Event> override {
    auto position = _state.position();
    auto possibleWatchpoints = _state.map.watchpoints(_state.map.findZone(position).zone);
    if (possibleWatchpoints.empty()) {
      return MovementOrientedRotationTree::act();
    }

    std::mt19937 gen(_rd());
    std::ranges::shuffle(possibleWatchpoints, gen);
    _targetAngle = Vector(position, possibleWatchpoints.back()).getAngle();
    return RotationTree::act();
  }

private:
  std::random_device _rd {};
};

class BackCheckingRotationTree : public RotationTree {
public:
  using RotationTree::RotationTree;

  auto act() -> std::unique_ptr<Event> override {
    auto orientation = _state.orientation().y;
    _targetAngle = orientation < 0.0f ? (orientation + 180.0f) : (orientation - 180.0f);
    return RotationTree::act();
  }
};

class MovementTree : public EnemyDependentTree<3> {
public:
  using EnemyDependentTree::EnemyDependentTree;

  auto act() -> std::unique_ptr<Event> override {
    auto position = _state.position();
    auto currentZone = _state.map.findZone(position);
    auto nextZone = _state.currentPath.back();
    auto movementVector = Vector {position, _state.nextPosition}.multiply(-_state.orientation().y);
    for (auto possibleTransitions = _state.map.possibleTransitions(currentZone.zone, nextZone);
         auto const& transition : possibleTransitions) {
      if (transition.transitionArea.contains(position)
          || transition.transitionArea.commonRegion(currentZone.zone.volume).contains(position)) {
        switch (transition.movement) {
          using enum Map::RequiredMovement;
          case JUMP: {
            return std::make_unique<JumpEvent>(movementVector, false);
          }
          case JUMP_AND_CROUCH: {
            return std::make_unique<JumpEvent>(movementVector, true);
          }
          default: {
            break;
          }
        }
      }
    }
    return std::make_unique<MovementEvent>(movementVector, 6000 * 30);
  }
};

class WeaponChoosingTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto act() -> std::unique_ptr<Event> override {
    char keyToPress {};
    if (_state.enemy != utils::sentinelBox) {
      if (_state.inventory().weapons()[0].weapon != NO_WEAPON
          && (_state.inventory().weapons()[0].ammo >= 5 || _state.inventory().weapons()[1].ammo < 5)) {
        keyToPress = '1';
      } else {
        keyToPress = '2';
      }
    } else {
      if (_iterationsHeld != 0) {
        --_iterationsHeld;
        return std::make_unique<EmptyEvent>();
      }
      if (_state.inventory().currentWeaponState() == Inventory::ActiveWeaponState::RELOADING
          || _state.inventory().currentWeapon().weapon == BOMB) {
        return std::make_unique<EmptyEvent>();
      }
      if (_state.map.findZone(_state.position()).name == Map::ZoneName::A_SITE
          && _state.round().bombState() != Round::BombState::PLANTED) {
        keyToPress = '5';
      } else {
        _iterationsHeld = persistanceCount;
        keyToPress = '3';
      }
    }
    return std::make_unique<KeyPressEvent>(keyToPress, 500);
  }

private:
  static constexpr auto persistanceCount = 5;
  int _iterationsHeld {};
};

class SituationalTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto evaluate() -> std::unique_ptr<Event> override {
    if (conditionsFulfilled()) {
      return DecisionTree::evaluate();
    }
    checkConditionInfluencers();
    return std::make_unique<EmptyEvent>();
  }
  [[nodiscard]] virtual auto conditionsFulfilled() const -> bool = 0;
  virtual auto checkConditionInfluencers() -> void {
    //empty on purpose
  }
};

class BombPlantingTree : public SituationalTree {
public:
  using SituationalTree::SituationalTree;

  [[nodiscard]] auto conditionsFulfilled() const -> bool override {
    return _state.map.findZone(_state.position()).name == Map::ZoneName::A_SITE
        && _state.round().bombState() != Round::BombState::PLANTED && _state.inventory().currentWeapon().weapon == BOMB;
  }

  auto act() -> std::unique_ptr<Event> override {
    return std::make_unique<MouseHoldEvent>(MouseButton::Button::LEFT_BUTTON, 6);
  }
};

class BuyingTree : public SituationalTree {
public:
  using SituationalTree::SituationalTree;

  [[nodiscard]] auto conditionsFulfilled() const -> bool override {
    return _buyingNeeded && _state.round().stage() == Round::Stage::START;
  }

  auto checkConditionInfluencers() -> void override {
    if (_state.round().stage() == Round::Stage::OVER) {
      _buyingNeeded = true;
    }
  }

  auto act() -> std::unique_ptr<Event> override {
    std::vector<BuyEvent::Item> itemsToBuy;
    if (_state.player().armor() == 0) {
      if (!_state.player().hasHelmet()) {
        itemsToBuy.push_back(BuyEvent::Item::KEVLAR_HELMET);
      } else {
        itemsToBuy.push_back(BuyEvent::Item::KEVLAR);
      }
    } else {
      if (_state.player().money() > 4000 || _state.player().armor() < 30) {
        itemsToBuy.push_back(BuyEvent::Item::KEVLAR);
      }
    }
    if (_state.inventory().weapons()[0].weapon == NO_WEAPON && _state.player().money() > 2700) {
      itemsToBuy.push_back(BuyEvent::Item::AK_47);
    }

    _buyingNeeded = false;

    log("Evaluated buy tree", OpState::INFO);
    return std::make_unique<BuyEvent>(itemsToBuy);
  }

private:
  bool _buyingNeeded {true};
};


} // namespace gabe
