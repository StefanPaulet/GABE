//
// Created by stefan on 5/12/24.
//

#pragma once

#include "types.hpp"
#include <utils/math/geometry/Geometry.hpp>
#include <vector>

namespace gabe {
enum class WeaponType { NONE, KNIFE, BOMB, AK_47, GLOCK, USPS, M4A4 };

struct WeaponSpray {
  std::vector<Point> points {};
  int state {0};
};

struct Weapon {
  WeaponType type {};
  bool automatic {true};
  float firerate {};
  WeaponSpray spray {};

  auto operator==(Weapon const& other) const -> bool { return type == other.type; }
  auto operator!=(Weapon const& other) const -> bool { return type != other.type; }

  [[nodiscard]] auto getSpray() -> Point {
    if (spray.points.empty()) {
      return {};
    }
    if (spray.state >= spray.points.size()) {
      spray.state = 0;
    }
    return spray.points[spray.state++];
  }

  [[nodiscard]] auto toString() const -> std::string {
    switch (type) {
      using enum WeaponType;
      case AK_47: {
        return "AK47";
      }
      case KNIFE: {
        return "KNIFE";
      }
      case BOMB: {
        return "BOMB";
      }
      case GLOCK: {
        return "GLOCK";
      }
      case USPS: {
        return "USPS";
      }
      case M4A4: {
        return "M4A4";
      }
      default: {
        break;
      }
    }
    return "unknown weapon type";
  }
};

Weapon const NO_WEAPON {WeaponType::NONE};
Weapon const KNIFE {WeaponType::KNIFE, false};
Weapon const BOMB {WeaponType::BOMB, false};
Weapon const AK_47 {
    WeaponType::AK_47, true, 600.0f,
    WeaponSpray {{Point {0, 25},   Point {0, 25},   Point {0, 25},   Point {0, 25},   Point {0, 25},
                  Point {3, 25},   Point {3, 25},   Point {3, 25},   Point {0, 25},   Point {-10, 5},
                  Point {-15, 5},  Point {-15, 25}, Point {-15, 5},  Point {-15, 5},  Point {-15, 5},
                  Point {15, 5},   Point {15, 5},   Point {15, 5},   Point {15, 5},   Point {15, 5},
                  Point {0, 2},    Point {0, 2},    Point {0, 2},    Point {0, 2},    Point {0, 2},
                  Point {-15, -5}, Point {-15, -5}, Point {-15, -5}, Point {-15, -5}, Point {-15, -5}}}};
Weapon const GLOCK {WeaponType::GLOCK, false, 400.0f, WeaponSpray {{}}};
Weapon const USPS {WeaponType::USPS, false, 352.94f, WeaponSpray {{}}};
Weapon const M4A4 {WeaponType::M4A4, true, 666.0f, WeaponSpray {{}}};
} // namespace gabe
