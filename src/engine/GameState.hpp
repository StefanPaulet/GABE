//
// Created by stefan on 5/8/24.
//

#pragma once

#include <cassert>
#include <mutex>
#include <utils/objectDetection/ObjectDetectionController.hpp>
#include <vector>

namespace gabe {

struct Shared {
  std::mutex lock;
};

struct Position {
  float x;
  float y;
  float z;
};

struct EnemyList {
  std::vector<utils::BoundingBox> enemies;
};

struct Orientation {
  float x;
  float y;
  float z;
};

struct SharedPosition : Position, Shared {
  SharedPosition() = default;
  explicit SharedPosition(Position const& other) : Position(other) {}

  auto& operator=(Position const& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
  }
};
struct SharedEnemyList : EnemyList, Shared {
  SharedEnemyList() = default;
  explicit SharedEnemyList(EnemyList const& other) : EnemyList(other) {}

  auto& operator=(EnemyList const& other) {
    enemies = other.enemies;
    return *this;
  }
};
struct SharedOrientation : Orientation, Shared {
  SharedOrientation() : Orientation() {}
  explicit SharedOrientation(Orientation const& other) : Orientation(other) {}

  auto& operator=(Orientation const& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
  }
};


class GameState {
public:
  enum class Properties { POSITION, ORIENTATION, ENEMIES };

  template <typename V> auto set(Properties property, V const& value) -> void {
    auto setter = [value]<typename T>(T& target) {
      std::lock_guard lg {target.lock};
      target = value;
    };

    switch (property) {
      using enum Properties;
      case POSITION: {
        if constexpr (std::is_assignable_v<std::remove_cvref_t<decltype(_position)>, V>) {
          setter(_position);
        }
        break;
      }
      case ORIENTATION: {
        if constexpr (std::is_assignable_v<std::remove_cvref_t<decltype(_orientation)>, V>) {
          setter(_orientation);
        }
        break;
      }
      case ENEMIES: {
        if constexpr (std::is_assignable_v<std::remove_cvref_t<decltype(_enemyList)>, V>) {
          setter(_enemyList);
        }
        break;
      }
      default: {
        assert(false && "Attempting to set unknown property");
      }
    }
  }

private:
  SharedPosition _position {};
  SharedOrientation _orientation {};
  SharedEnemyList _enemyList {};
};
} // namespace gabe