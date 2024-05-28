//
// Created by stefan on 2/8/24.
//

#pragma once

#include <cmath>
#include <compare>

namespace gabe {
static constexpr auto expectedScreenWidth = 1920;
static constexpr auto expectedScreenHeight = 1080;
static constexpr auto screenHeightOffset = 64;

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

struct Line {
  Line() = default;
  Line(Line const&) = default;
  Line(Line&&) noexcept = default;
  Line(float m, float b) : m {m}, b {b} {}
  Line(Position const& pos1, Position const& pos2) :
      m {(pos2.y - pos1.y) / (pos2.x - pos1.x)}, b {pos2.y - m * pos2.x} {}

  [[nodiscard]] auto value(Position const& point) const -> float { return m * point.x + b; }
  [[nodiscard]] auto inverse(Position const& point) const -> float { return (point.y - b) / m; }

  float m;
  float b;
};

struct Volume {
  enum class Projection { X, Y, Z };
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
        std::max(std::min(firstCorner.y, secondCorner.y), std::min(other.firstCorner.y, other.secondCorner.y))};
    Position resultSecondCorner = {
        std::min(std::max(firstCorner.x, secondCorner.x), std::max(other.firstCorner.x, other.secondCorner.x)),
        std::min(std::max(firstCorner.y, secondCorner.y), std::max(other.firstCorner.y, other.secondCorner.y))};
    return {resultFirstCorner, resultSecondCorner};
  }

  [[nodiscard]] auto containsLine(Line const& line) const -> bool {
    auto firstVerticalIntersection = Position {firstCorner.x, line.value(firstCorner), 0};
    auto secondVerticalIntersection = Position {secondCorner.x, line.value(secondCorner), 0};
    if (containsProjection(firstVerticalIntersection, Projection::Y)
        && containsProjection(secondVerticalIntersection, Projection::Y)) {
      return true;
    }
    auto firstHorizontalIntersection = Position {line.inverse(firstCorner), firstCorner.y, 0};
    auto secondHorizontalIntersection = Position {line.inverse(secondCorner), secondCorner.y, 0};
    return containsProjection(firstHorizontalIntersection, Projection::X)
        && containsProjection(secondHorizontalIntersection, Projection::X);
  }
};

} // namespace gabe
