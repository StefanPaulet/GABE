//
// Created by stefan on 5/9/24.
//

#pragma once

#include "../windowController/Synchronizer.hpp"
#include "Exceptions.hpp"
#include "GameState.hpp"
#include "event/Event.hpp"
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

  virtual auto randomDecision() -> Decision = 0;
  virtual auto act() -> std::unique_ptr<Event> = 0;
  virtual auto postEvaluation() -> void {}

  auto evaluate() -> std::unique_ptr<Event> {
    if (_children.empty()) {
      return act();
    }

    std::random_device rd {};
    auto targetValue = std::uniform_real_distribution<float> {}(rd);
    auto chosenValue = .0f;
    for (auto const& e : _children) {
      chosenValue += e.weight;
      if (chosenValue > targetValue) {
        return e.decision->evaluate();
      }
    }
    return randomDecision().decision->evaluate();
  }

protected:
  GameState& _state;

private:
  std::vector<Decision> _children;
};

class ImageCapturingTree : public DecisionTree {
public:
  ImageCapturingTree(GameState& gameState, Synchronizer& synchronizer) :
      DecisionTree {gameState}, _synchronizer {synchronizer} {
    _image = new unsigned char[expectedScreenHeight * expectedScreenWidth * 3];
  }

  ~ImageCapturingTree() override { delete[] _image; }

  auto randomDecision() -> Decision override { return {}; }
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

  auto randomDecision() -> Decision override { return {}; }
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
} // namespace gabe
