//
// Created by stefan on 5/9/24.
//

#pragma once

#include "Exceptions.hpp"
#include "GameState.hpp"
#include "Movement.hpp"
#include "Path.hpp"
#include "event/Event.hpp"
#include "synchronizer/Synchronizer.hpp"
#include "utils/objectDetection/ObjectDetectionController.hpp"
#include "utils/positionReader/PositionReader.hpp"
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

class ConditionalTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto evaluate() -> std::unique_ptr<Event> override {
    if (!evaluationNeeded() && _lastActiveChild != nullptr) {
      return _lastActiveChild->decision->evaluate();
    }

    if (_children.empty()) {
      return act();
    }
    auto& child = selectChild();
    _lastActiveChild = &child;
    return child.decision->evaluate();
  }
  virtual auto evaluationNeeded() -> bool = 0;

protected:
  Decision* _lastActiveChild;
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
      log(std::format("Detected enemy between x1={}, y1={}, x2={}, y2={}", _state.enemy.topLeft.x,
                      _state.enemy.topLeft.y, _state.enemy.bottomRight.x, _state.enemy.bottomRight.y),
          OpState::INFO);
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
    if (_state.enemy == utils::sentinelBox) {
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
    log(std::format("Decided to use slow shooting of type {}", aimType == ShootEvent::AimType::FLICK ? "flick" : "tap"),
        OpState::INFO);
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
    auto bulletCount = 5;
    log("Decided to use spray shooting", OpState::INFO);
    return std::make_unique<SprayEvent>(shootingPoint, bulletCount, _state.weapon);
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

class DestinationChoosingTree : public ConditionalTree {
public:
  using ConditionalTree::ConditionalTree;

  auto evaluate() -> std::unique_ptr<Event> override {
    if (evaluationNeeded() || _lastActiveChild == nullptr) {
      _state.targetZone.name = Map::ZoneName::A_SITE;
      log("Evaluating destination choosing tree to set destination to " + _state.targetZone.toString(), OpState::INFO);
    }
    return ConditionalTree::evaluate();
  }

  auto evaluationNeeded() -> bool override {
    auto currentZone = _state.map.findZone(_state.position());
    return currentZone.name == _state.targetZone.name;
  }
};

template <typename PathChoosingPolicy> class PathChoosingTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto act() -> std::unique_ptr<Event> override {
    _policy.getPath(_state.map.findZone(_state.position()).name, _state.targetZone.name);
    _state.currentPath = _policy.path;
    return std::make_unique<EmptyEvent>();
  }

private:
  PathChoosingPolicy _policy {_state.map};
};

template <typename MovementPolicy> class MovementTree : public DecisionTree {
public:
  using DecisionTree::DecisionTree;

  auto act() -> std::unique_ptr<Event> override {
    auto position = _state.position();
    auto currentZone = _state.map.findZone(position);
    auto nextPoint = MovementPolicy {_state.map}.getNextPoint(position, currentZone.zone, _state.currentPath);
    auto angle = _state.orientation().y - Vector(position, nextPoint).getAngle();
    if (std::abs(angle) > 180.0f) {
      angle = 360 - angle * (angle < 0.0f ? -1 : 1);
    }
    if (std::abs(angle) > 5.0f) {
      return std::make_unique<RotationEvent>(angle, RotationEvent::Axis::OX);
    }
    for (auto possibleTransitions = _state.map.possibleTransitions(currentZone.zone, _state.targetZone.zone);
         auto const& transition : possibleTransitions) {
      if (transition.transitionArea.contains(position)
          || transition.transitionArea.commonRegion(currentZone.zone.volume).contains(position)) {
        switch (transition.movement) {
          using enum Map::RequiredMovement;
          case JUMP: {
            return std::make_unique<JumpEvent>(Vector {0, 1}, false);
          }
          case JUMP_AND_CROUCH: {
            return std::make_unique<JumpEvent>(Vector {0, 1}, true);
          }
          default: {
            break;
          }
        }
      }
    }
    return std::make_unique<MovementEvent>(Vector {0, 1}, 6000 * 10);
  }

private:
  Map::NamedZone _lastZone {};
};
} // namespace gabe
