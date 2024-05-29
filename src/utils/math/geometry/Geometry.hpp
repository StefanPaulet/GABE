//
// Created by stefan on 5/28/24.
//

#pragma once

#include <cmath>

namespace gabe {
struct Point {
  Point() = default;
  Point(Point const&) = default;
  constexpr Point(int x, int y) : x {x}, y {y} {}

  auto operator+(Point const& other) const -> Point { return {x + other.x, y + other.y}; }
  auto operator+=(Point const& other) -> Point& {
    x += other.x;
    y += other.y;
    return *this;
  }

  auto operator-(Point const& other) const -> Point { return {x - other.x, y - other.y}; }
  auto operator-=(Point const& other) -> Point& {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  auto operator/(int value) const -> Point { return {x / value, y / value}; }
  auto operator/=(int value) -> Point& {
    x /= value;
    y /= value;
    return *this;
  }

  auto operator/(Point value) const -> Point { return {x / value.x, y / value.y}; }
  auto operator/=(Point value) -> Point& {
    x /= value.x;
    y /= value.y;
    return *this;
  }

  auto operator%(int value) const -> Point { return {x % value, y % value}; }
  auto operator%=(int value) -> Point& {
    x %= value;
    y %= value;
    return *this;
  }

  auto operator*(int value) const -> Point { return {x * value, y * value}; }
  auto operator*=(int value) -> Point& {
    x *= value;
    y *= value;
    return *this;
  }

  auto operator*(double value) const -> Point { return {static_cast<int>(x * value), static_cast<int>(y * value)}; }
  auto operator*=(double value) -> Point& {
    x = static_cast<int>(x * value);
    y = static_cast<int>(y * value);
    return *this;
  }

  [[nodiscard]] auto abs() const -> Point { return {std::abs(x), std::abs(y)}; }

  auto operator<=>(Point const& other) const = default;

  int x;
  int y;
};

struct Position {
  float x;
  float y;
  float z;

  auto operator<=>(Position const& other) const = default;
  auto empty() const -> bool { return x == 0 && y == 0 && z == 0; }
};

struct Orientation {
  float x;
  float y;
  float z;
};

struct Vector {
  Vector() = default;
  Vector(Vector const&) = default;
  Vector(Vector&&) = default;
  Vector(float x, float y) : x {x}, y {y} {}
  Vector(Position const& pos1, Position const& pos2) : x {pos2.x - pos1.x}, y {pos2.y - pos1.y} {}

  [[nodiscard]] auto getAngle() const -> float {
    return std::atanf(y / x) * 180.0f / std::numbers::pi_v<float> + (x < 0 ? 180.0f : .0f);
  }

  float x;
  float y;
};

struct Volume {
  enum class Projection { X, Y, Z };
  Position firstCorner;
  Position secondCorner;

  auto operator<=>(Volume const& other) const = default;

  auto empty() const -> bool { return firstCorner.empty() && secondCorner.empty(); }

  [[nodiscard]] auto intersects(Volume const& other) const -> bool {
    auto firstAlternativeCorner = Position {firstCorner.x, secondCorner.y};
    auto secondAlternativeCorner = Position {secondCorner.x, firstCorner.y};

    auto otherFirstAlternativeCorner = Position {other.firstCorner.x, other.secondCorner.y};
    auto otherSecondAlternativeCorner = Position {other.secondCorner.x, other.firstCorner.y};

    return containsInXY(other.firstCorner) || containsInXY(other.secondCorner)
        || containsInXY(otherFirstAlternativeCorner) || containsInXY(otherSecondAlternativeCorner)
        || other.containsInXY(firstCorner) || other.containsInXY(secondCorner)
        || other.containsInXY(firstAlternativeCorner) || other.containsInXY(secondAlternativeCorner);
  }

  [[nodiscard]] auto center() const -> Position {
    return {(firstCorner.x + secondCorner.x) / 2, (firstCorner.y + secondCorner.y) / 2,
            (firstCorner.z + secondCorner.z) / 2};
  }

  [[nodiscard]] auto distance(Position const& position) const -> float {
    Position closest = closestPoint(position);
    auto distance = Position {closest.x - position.x, closest.y - position.y, closest.z - position.z};
    return std::sqrt(distance.x * distance.x + distance.y * distance.y + distance.z * distance.z);
  }

  [[nodiscard]] auto distance(Volume const& other) const -> float { return distance(other.closestPoint(firstCorner)); }

  [[nodiscard]] auto closestPoint(Position const& position) const -> Position {
    Position closest {};
    auto localClosest = [](float val, float t1, float t2) {
      if (val < std::min(t1, t2) || val > std::max(t1, t2)) {
        return (std::abs(val - t1) < std::abs(val - t2)) ? t1 : t2;
      }
      return val;
    };
    closest.x = localClosest(position.x, firstCorner.x, secondCorner.x);
    closest.y = localClosest(position.y, firstCorner.y, secondCorner.y);
    closest.z = localClosest(position.z, firstCorner.z, secondCorner.z);
    return closest;
  }

  [[nodiscard]] auto containsProjection(Position const& position, Projection projection) const -> bool {
    auto inside = [](float target, float t1, float t2) {
      return target >= std::min(t1, t2) && target <= std::max(t1, t2);
    };
    switch (projection) {
      using enum Projection;
      case X: {
        return inside(position.x, firstCorner.x, secondCorner.x);
      }
      case Y: {
        return inside(position.y, firstCorner.y, secondCorner.y);
      }
      case Z: {
        return inside(position.z, firstCorner.z, secondCorner.z);
      }
    }
    return false;
  }

  [[nodiscard]] auto perpendicularTo(Position const& position) const -> bool {
    return containsProjection(position, Projection::X) || containsProjection(position, Projection::Y);
  }

  [[nodiscard]] auto containsInXY(Position const& position) const -> bool {
    return containsProjection(position, Projection::X) && containsProjection(position, Projection::Y);
  }

  [[nodiscard]] auto contains(Position const& position) const -> bool {
    return containsInXY(position) && containsProjection(position, Projection::Z);
  }

  [[nodiscard]] auto commonRegion(Volume const& other) const -> Volume {
    Position resultFirstCorner = {
        std::max(std::min(firstCorner.x, secondCorner.x), std::min(other.firstCorner.x, other.secondCorner.x)),
        std::max(std::min(firstCorner.y, secondCorner.y), std::min(other.firstCorner.y, other.secondCorner.y)),
        std::max(std::min(firstCorner.z, secondCorner.z), std::min(other.firstCorner.z, other.secondCorner.z))};
    Position resultSecondCorner = {
        std::min(std::max(firstCorner.x, secondCorner.x), std::max(other.firstCorner.x, other.secondCorner.x)),
        std::min(std::max(firstCorner.y, secondCorner.y), std::max(other.firstCorner.y, other.secondCorner.y)),
        std::min(std::max(firstCorner.z, secondCorner.z), std::max(other.firstCorner.z, other.secondCorner.z))};
    return {resultFirstCorner, resultSecondCorner};
  }

  [[nodiscard]] auto join(Volume const& other) const -> Volume {
    auto min = [](float v1, float v2, float v3, float v4) { return std::min(std::min(v1, v2), std::min(v3, v4)); };
    auto max = [](float v1, float v2, float v3, float v4) { return std::max(std::max(v1, v2), std::max(v3, v4)); };
    auto newFirstCorner = Position {min(firstCorner.x, secondCorner.x, other.firstCorner.x, other.secondCorner.x),
                                    min(firstCorner.y, secondCorner.y, other.firstCorner.y, other.secondCorner.y),
                                    min(firstCorner.z, secondCorner.z, other.firstCorner.z, other.secondCorner.z)};
    auto newSecondCorner = Position {max(firstCorner.x, secondCorner.x, other.firstCorner.x, other.secondCorner.x),
                                     max(firstCorner.y, secondCorner.y, other.firstCorner.y, other.secondCorner.y),
                                     max(firstCorner.z, secondCorner.z, other.firstCorner.z, other.secondCorner.z)};
    return {newFirstCorner, newSecondCorner};
  }
};
} // namespace gabe