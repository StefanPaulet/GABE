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
  virtual auto postEvaluation() -> void {}

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

class ShootingTree : public DecisionTree {
public:
  ShootingTree(GameState& gameState, std::string const& scriptPath) :
      DecisionTree {gameState}, _objectDetectionController {scriptPath} {}

  auto act() -> std::unique_ptr<Event> override {
    auto* image = _state.image().data;

    if (auto enemyList = _objectDetectionController.analyzeImage(image); !enemyList.empty()) {
      auto shootingPoint = (enemyList[0].topLeft + (enemyList[0].bottomRight - enemyList[0].topLeft) / Point {2, 5})
          - Point {expectedScreenWidth, expectedScreenHeight} / 2;
      ShootEvent::AimType aimType;
      if (std::abs(shootingPoint.x) > 50 || std::abs(shootingPoint.y) > 50) {
        aimType = ShootEvent::AimType::FLICK;
        shootingPoint *= 2;
      } else {
        aimType = ShootEvent::AimType::TAP;
      }
      log(std::format("Detected enemy at x={}, y={}", shootingPoint.x, shootingPoint.y), OpState::INFO);
      return std::make_unique<ShootEvent>(shootingPoint, aimType);
    }
    return std::make_unique<EmptyEvent>();
  }

  auto postEvaluation() -> void override { usleep(100000); }

private:
  utils::ObjectDetectionController _objectDetectionController;
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
    auto nextPoint = MovementPolicy().getNextPoint(position, currentZone.zone, _state.currentPath);
    auto angle = _state.orientation().y - Vector(position, nextPoint).getAngle();
    if (std::abs(angle) > 180.0f) {
      angle = 360 - angle * (angle < 0.0f ? -1 : 1);
    }
    if (std::abs(angle) > 5.0f) {
      return std::make_unique<RotationEvent>(angle, RotationEvent::Axis::OX);
    }
    for (auto possibleTransitions = _state.map.possibleTransitions(currentZone.zone, _state.targetZone.zone);
         auto const& transition : possibleTransitions) {
      if (transition.transitionArea.contains(position)) {
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
