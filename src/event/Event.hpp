//
// Created by stefan on 3/28/24.
//

#pragma once
#include "Exceptions.hpp"
#include <types.hpp>
#include <utils/logger/Logger.hpp>

namespace gabe {

struct Point {
  Point() = default;
  Point(Point const&) = default;
  Point(int x, int y) : x {x}, y {y} {}

  auto operator+(Point const& other) const -> Point { return {x + other.x, y + other.y}; }
  auto operator+=(Point const& other) -> Point& {
    *this = *this + other;
    return *this;
  }
  auto operator/(int value) const -> Point { return {x / value, y / value}; }
  int x;
  int y;
};

class Event {
public:
  virtual auto solve(Display*, Window) noexcept(false) -> void = 0;
  virtual ~Event() = default;
  auto translateCoordinates(Display* display, Window window) -> Point {
    int targetX;
    int targetY;
    Window child;
    XTranslateCoordinates(display, window, DefaultRootWindow(display), 0, 0, &targetX, &targetY, &child);
    return {targetX, targetY};
  }
};

class MouseMoveEvent : public Event {
public:
  MouseMoveEvent() = default;
  MouseMoveEvent(MouseMoveEvent const&) = default;
  MouseMoveEvent(MouseMoveEvent&&) noexcept = default;
  explicit MouseMoveEvent(Point const& point) : _point {point} {}

  auto solve(Display* display, Window window) noexcept(false) -> void override {
    _point = _point + translateCoordinates(display, window);
    XSetInputFocus(display, window, RevertToParent, CurrentTime);
    XFlush(display);

    XSelectInput(display, window, PointerMotionMask);
    XFlush(display);

    XTestFakeMotionEvent(display, -1, _point.x, _point.y, CurrentTime);
    XFlush(display);

    log(std::format("Treated move mouse event at x={}, y={}", _point.x, _point.y), OpState::INFO);
  }

private:
  Point _point {};
};

class StrafeEvent : public Event {
public:
  StrafeEvent() = default;
  StrafeEvent(StrafeEvent const&) = default;
  StrafeEvent(StrafeEvent&&) noexcept = default;
  explicit StrafeEvent(Point const& movement) : _totalMovement {movement} {}

  auto solve(Display* display, Window window) noexcept(false) -> void override {
    Window rootWindow;
    Point rootPoint {};
    Point windowPoint {};
    unsigned int mask;

    rootWindow = XRootWindow(display, DefaultScreen(display));

    XQueryPointer(display, rootWindow, &rootWindow, &window, &rootPoint.x, &rootPoint.y, &windowPoint.x, &windowPoint.y,
                  &mask);

    XSelectInput(display, window, PointerMotionMask);
    XFlush(display);


    Point p = windowPoint;
    for (auto idx = 0; idx < itCount; ++idx) {
      p += _totalMovement / itCount;
      XSelectInput(display, window, PointerMotionMask);
      XFlush(display);

      XTestFakeMotionEvent(display, -1, p.x, p.y, CurrentTime);
      XFlush(display);
      usleep(100);
    }

    log(std::format("Treated strafe mouse event at x={}, y={}", windowPoint.x, windowPoint.y), OpState::INFO);
  }

private:
  static constexpr auto itCount = 50;
  Point _totalMovement {};
};
} // namespace gabe