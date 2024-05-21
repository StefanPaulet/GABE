//
// Created by stefan on 5/12/24.
//

#pragma once

namespace gabe {
enum class WeaponType { AK_47, GLOCK, USPS, M4A4 };

struct WeaponSpray {
  std::vector<Point> points {};
  int state {0};
};

struct Weapon {
  WeaponType const type {};
  float const firerate {};
  WeaponSpray spray {};

  [[nodiscard]] auto getSpray() -> Point {
    if (spray.state > spray.points.size()) {
      spray.state = 0;
    }
    return spray.points[spray.state++];
  }
};

Weapon const AK_47 {WeaponType::AK_47, 600.0f,
                    WeaponSpray {{Point {0, 25}, Point {0, 25}, Point {0, 25}, Point {0, 25}, Point {0, 25},
                                  Point {0, 25}, Point {0, 25}, Point {0, 25}, Point {0, 25}, Point {0, 25}}}};
Weapon const GLOCK {WeaponType::GLOCK, 5.0f, WeaponSpray {{}}};
Weapon const USPS {WeaponType::USPS, 5.0f, WeaponSpray {{}}};
Weapon const M4A4 {WeaponType::M4A4, 666.0f, WeaponSpray {{}}};
} // namespace gabe