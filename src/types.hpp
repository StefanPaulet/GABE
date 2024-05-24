//
// Created by stefan on 2/8/24.
//

#pragma once

#include <cmath>
#include <compare>

namespace gabe {
using uint8 = unsigned char;
using uint16 = unsigned short int;
using uint32 = unsigned int;
using uint64 = unsigned long int;

using Size = uint64;

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

  auto operator<=>(Point const& other) const = default;

  int x;
  int y;
};

struct Position {
  float x;
  float y;
  float z;

  auto operator<=>(Position const& other) const = default;
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
  Vector(Position const& pos1, Position const& pos2) : x {pos2.x - pos1.x}, y {pos2.y - pos1.y} { normalize(); }

  auto normalize() -> void {
    auto sum = std::sqrt(x * x + y * y);
    x = x / sum;
    y = y / sum;
  }

  [[nodiscard]] auto getAngle() const -> float { return std::atanf(y / x) * 180 / std::numbers::pi; }

  float x;
  float y;
};

struct Volume {
  Position firstCorner;
  Position secondCorner;

  auto operator<=>(Volume const& other) const = default;

  [[nodiscard]] auto center() const -> Position {
    return {(firstCorner.x + secondCorner.x) / 2, (firstCorner.y + secondCorner.y) / 2,
            (firstCorner.z + secondCorner.z) / 2};
  }

  [[nodiscard]] auto distance(Position const& position) const -> float {
    Position distance = closestPoint(position);
    return std::sqrt(distance.x * distance.x + distance.y * distance.y + distance.z * distance.z);
  }

  [[nodiscard]] auto closestPoint(Position const& position) const -> Position {
    Position distance {};
    auto localDistance = [](float val, float t1, float t2) {
      if (val < std::min(t1, t2) || val > std::max(t1, t2)) {
        return std::min(std::abs(val - t1), std::abs(val - t2));
      }
      return .0f;
    };
    distance.x = localDistance(position.x, firstCorner.x, secondCorner.x);
    distance.y = localDistance(position.y, firstCorner.y, secondCorner.y);
    distance.z = localDistance(position.z, firstCorner.z, secondCorner.z);
    return distance;
  }
};

} // namespace gabe
