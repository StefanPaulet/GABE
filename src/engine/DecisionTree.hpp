//
// Created by stefan on 5/9/24.
//

#pragma once

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
  DecisionTree() = default;
  DecisionTree(DecisionTree const&) = delete;
  DecisionTree(DecisionTree&&) noexcept = default;
  explicit DecisionTree(std::vector<Decision>&& list, GameState& state) : _state(state) {
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

  virtual auto random_decision() -> Decision = 0;
  virtual auto act() -> std::unique_ptr<Event> = 0;

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
    return random_decision().decision->evaluate();
  }

protected:
  GameState& _state;

private:
  std::vector<Decision> _children;
};
} // namespace gabe