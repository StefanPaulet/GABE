//
// Created by stefan on 2/8/24.
//

#pragma once

namespace gabe {
using uint8 = unsigned char;
using uint16 = unsigned short int;
using uint32 = unsigned int;
using uint64 = unsigned long int;

using Size = uint64;

struct Point {
  Point() = default;
  Point(Point const&) = default;
  Point(int x, int y) : x {x}, y {y} {}

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

  auto operator*(int value) const -> Point { return {x * value, y * value}; }
  auto operator*=(int value) -> Point& {
    x *= value;
    y *= value;
    return *this;
  }

  auto operator==(Point const& other) const -> bool = default;

  int x;
  int y;
};
} // namespace gabe
