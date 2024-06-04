//
// Created by stefan on 3/28/24.
//

#pragma once

#include "../engine/Weapon.hpp"
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <cassert>
#include <condition_variable>
#include <cstring>
#include <jpeglib.h>
#include <types.hpp>
#include <unistd.h>
#include <utils/logger/Logger.hpp>

namespace gabe {
static constexpr auto dpiScalingFactor = 1;


static auto saveImage(std::string_view filename, unsigned char* data, int width, int height) -> void {
  struct jpeg_compress_struct cinfo {};
  struct jpeg_error_mgr jerr {};

  JSAMPROW row_pointer;

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);
  auto* outfile = fopen(filename.data(), "wb");

  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);

  jpeg_start_compress(&cinfo, TRUE);

  auto const row_stride = width * 3;

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer = &data[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  }
  fclose(outfile);
}


class Event {
public:
  virtual auto solve(Display* display, Window window) -> void = 0;
  virtual ~Event() = default;
  auto translateCoordinates(Display* display, Window window) -> Point {
    int targetX;
    int targetY;
    Window child;
    XTranslateCoordinates(display, window, DefaultRootWindow(display), 0, 0, &targetX, &targetY, &child);
    return {targetX, targetY};
  }
};

class EmptyEvent : public Event {
public:
  EmptyEvent() = default;
  EmptyEvent(EmptyEvent const&) = default;
  EmptyEvent(EmptyEvent&&) noexcept = default;

  auto solve(Display* display, Window window) -> void override {
    //empty on purpose
  }
};

class MouseScanEvent : public Event {
public:
  MouseScanEvent() = default;
  MouseScanEvent(MouseScanEvent const&) = default;
  MouseScanEvent(MouseScanEvent&&) noexcept = default;

  auto solve(Display* display, Window window) -> void override {
    Window childWindow;
    int rootX, rootY, winX, winY;
    unsigned int mask;

    XQueryPointer(display, window, &window, &childWindow, &rootX, &rootY, &winX, &winY, &mask);
    log(std::format("Treated mouse scan event; x={}, y={}", rootX, rootY), OpState::INFO);
  }
};

class MouseMoveEvent : public Event {
public:
  MouseMoveEvent() = default;
  MouseMoveEvent(MouseMoveEvent const&) = default;
  MouseMoveEvent(MouseMoveEvent&&) noexcept = default;
  explicit MouseMoveEvent(Point const& point) : _point {point} {}

  auto solve(Display* display, Window window) -> void override {
    _point *= dpiScalingFactor;

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
  StrafeEvent(Point const& movement, int iterationCount, int strafeSpeed) :
      _itCount {iterationCount}, _strafeSpeed {strafeSpeed}, _totalMovement {movement} {}

  auto solve(Display* display, Window window) -> void override {
    XWindowAttributes attr;
    int revert;
    XGetInputFocus(display, &window, &revert);
    XGetWindowAttributes(display, window, &attr);

    XSelectInput(display, window, PointerMotionMask);
    XFlush(display);

    Point p {};
    for (auto idx = 0; idx < _itCount; ++idx) {
      p = _totalMovement / _itCount;
      XSelectInput(display, window, PointerMotionMask);
      XFlush(display);

      XTestFakeMotionEvent(display, -1, p.x, p.y, CurrentTime);
      XFlush(display);
      usleep(_strafeSpeed);
    }

    _totalMovement = _totalMovement / _itCount * _itCount;
  }

private:
  static constexpr auto sleepTime = 5000;
  int _itCount {};
  int _strafeSpeed {};
  Point _totalMovement {};
};

class FullStrafeEvent : public Event {
public:
  FullStrafeEvent() = default;
  FullStrafeEvent(FullStrafeEvent const&) = default;
  FullStrafeEvent(FullStrafeEvent&&) noexcept = default;
  FullStrafeEvent(Point const& movement, int large, int small, int sleep) :
      _totalMovement {movement}, _largeSteps {large}, _smallSteps {small}, _sleepTime {sleep} {}

  auto solve(Display* display, Window window) -> void override {
    StrafeEvent(_totalMovement, _largeSteps, _sleepTime).solve(display, window);
    StrafeEvent(_totalMovement % _largeSteps, _smallSteps, _sleepTime).solve(display, window);
  }

private:
  Point _totalMovement {};
  int _largeSteps {};
  int _smallSteps {};
  int _sleepTime {};
};

class ScreenshotEvent : public Event {
public:
  ScreenshotEvent() = delete;
  ScreenshotEvent(ScreenshotEvent const&) = default;
  ScreenshotEvent(ScreenshotEvent&&) noexcept = default;

  explicit ScreenshotEvent(unsigned char* targetData) : _pTargetData(targetData) {}

  auto solve(Display* display, Window window) -> void override {
    XWindowAttributes attr;
    XGetWindowAttributes(display, window, &attr);

    XImage* img = XGetImage(display, window, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);
    char const* data = img->data;
    assert(attr.width == expectedScreenWidth && attr.height == expectedScreenHeight
           && "Window does not match expected sizes");

    int bytesPerPixel = img->bits_per_pixel / 8;
    int bytesPerLine = img->bytes_per_line;
    for (int y = 0; y < attr.height; ++y) {
      for (int x = 0; x < attr.width; ++x) {
        int offset = y * bytesPerLine + x * bytesPerPixel;
        int jpgOffset = y * attr.width * 3 + x * 3;

        _pTargetData[jpgOffset + 0] = data[offset + 2];
        _pTargetData[jpgOffset + 1] = data[offset + 1];
        _pTargetData[jpgOffset + 2] = data[offset + 0];
      }
    }
    XDestroyImage(img);
  }

private:
  unsigned char* _pTargetData;
};

struct MouseButton {
  enum class Button : int {
    LEFT_BUTTON = Button1,
    RIGHT_BUTTON = Button2,
    WHEEL_PRESSED = Button3,
    WHEEL_UP = Button4,
    WHEEL_DOWN = Button5
  };

  MouseButton() = default;
  MouseButton(MouseButton const&) = default;
  MouseButton(MouseButton&&) noexcept = default;
  explicit MouseButton(Button value) : button {value} {}

  [[nodiscard]] constexpr auto toString() const -> std::string {
    switch (button) {
      using enum MouseButton::Button;
      case LEFT_BUTTON: return "left click";
      case RIGHT_BUTTON: return "right click";
      case WHEEL_PRESSED: return "middle click";
      case WHEEL_UP: return "wheel up";
      case WHEEL_DOWN: return "wheel down";
    }
    assert(false && "Button type undefined");
    return "<undefined button type>";
  };

  Button button {};
};

class MouseActionEvent : public Event {
public:
  MouseActionEvent() = default;
  MouseActionEvent(MouseActionEvent const&) = default;
  MouseActionEvent(MouseActionEvent&&) noexcept = default;
  MouseActionEvent(MouseButton::Button val, bool press) : _buttonType {val}, _press {press} {}

  auto solve(Display* display, Window window) -> void override {
    auto buttonId = static_cast<std::underlying_type_t<MouseButton::Button>>(_buttonType.button);
    XTestFakeButtonEvent(display, buttonId, _press, CurrentTime);
    XFlush(display);
    usleep(sleepTime);
  }

private:
  static constexpr auto sleepTime = 100;
  MouseButton _buttonType {};
  bool _press {};
};

class MouseClickEvent : public Event {
public:
  MouseClickEvent() = default;
  MouseClickEvent(MouseClickEvent const&) = default;
  MouseClickEvent(MouseClickEvent&&) noexcept = default;
  explicit MouseClickEvent(MouseButton::Button val) : _buttonType {val} {}

  auto solve(Display* display, Window window) -> void override {
    MouseActionEvent(_buttonType.button, true).solve(display, window);
    MouseActionEvent(_buttonType.button, false).solve(display, window);
    log("Treated mouse click event of type " + _buttonType.toString(), OpState::INFO);
  }

private:
  MouseButton _buttonType {};
};

class ShootEvent : public Event {
public:
  enum class AimType { FLICK, TAP };

  ShootEvent() = default;
  ShootEvent(ShootEvent const&) = default;
  ShootEvent(ShootEvent&&) noexcept = default;
  ShootEvent(Point const& movement, AimType aimType) : _totalMovement {movement}, _aimType {aimType} {}

  auto solve(Display* display, Window window) -> void override {
    switch (_aimType) {
      using enum AimType;
      case FLICK: {
        constexpr auto largeSteps = 50;
        constexpr auto smallSteps = 2;
        FullStrafeEvent(_totalMovement, largeSteps, smallSteps, 2750).solve(display, window);
        MouseClickEvent(MouseButton::Button::LEFT_BUTTON).solve(display, window);
        break;
      }
      case TAP: {
        StrafeEvent(_totalMovement, 1, 250).solve(display, window);
        MouseClickEvent(MouseButton::Button::LEFT_BUTTON).solve(display, window);
        usleep(1500);
        break;
      }
    }

    auto toString = [*this] {
      switch (_aimType) {
        using enum AimType;
        case FLICK: {
          return "flick";
        }
        case TAP: {
          return "tap";
        }
        default: {
          return "<unknown event>";
        }
      }
    };
    log(std::format("Treated shoot event of type {} at x={} y={}", toString(), _totalMovement.x, _totalMovement.y),
        OpState::INFO);
  }

private:
  Point _totalMovement {};
  AimType _aimType {};
};

class SprayEvent : public Event {
public:
  SprayEvent() = default;
  SprayEvent(SprayEvent const&) = default;
  SprayEvent(SprayEvent&&) noexcept = default;
  SprayEvent(Point const& startPoint, int bulletCount, Weapon const& weapon) :
      _startPoint {startPoint}, _bulletCount {bulletCount}, _weapon {weapon} {}

  auto solve(Display* display, Window window) -> void override {
    StrafeEvent(_startPoint, 1, 250).solve(display, window);
    if (_weapon.automatic) {
      MouseActionEvent(MouseButton::Button::LEFT_BUTTON, true).solve(display, window);
      for (auto idx = 0; idx < _bulletCount; ++idx) {
        StrafeEvent(_weapon.getSpray(), 1, 0).solve(display, window);
        usleep(static_cast<int>(55000000 / _weapon.firerate));
      }
      MouseActionEvent(MouseButton::Button::LEFT_BUTTON, false).solve(display, window);
    } else {
      for (auto idx = 0; idx < _bulletCount; ++idx) {
        MouseClickEvent(MouseButton::Button::LEFT_BUTTON).solve(display, window);
        usleep(static_cast<int>(60000000 / _weapon.firerate));
      }
    }
    log(std::format("Treated spray event at x={} y={}", _startPoint.x, _startPoint.y), OpState::INFO);
  }

private:
  Point _startPoint {};
  int _bulletCount {};
  Weapon _weapon {};
};

class KeyActionEvent : public Event {
public:
  KeyActionEvent() = default;
  KeyActionEvent(KeyActionEvent const&) = default;
  KeyActionEvent(KeyActionEvent&&) noexcept = default;
  KeyActionEvent(char key, int sleepTime, bool press) : _key {key}, _sleepTime {sleepTime}, _press {press} {}

  auto solve(Display* display, Window window) -> void override {
    KeySym keyCode {static_cast<KeySym>(_key)};
    if (static_cast<int>(_key) >= 0x20) {
      keyCode = XKeysymToKeycode(display, _key);
    } else {
      switch (_key) {
        case '\n': {
          keyCode = XKeysymToKeycode(display, XK_Return);
          break;
        }
        case ' ': {
          keyCode = XKeysymToKeycode(display, XK_space);
          break;
        }
        case 27: {
          keyCode = XKeysymToKeycode(display, XK_Escape);
          break;
        }
      }
    }
    XTestFakeKeyEvent(display, keyCode, _press, CurrentTime);
    XSync(display, False);
    usleep(_sleepTime);
  }

private:
  char _key {};
  int _sleepTime {};
  bool _press {};
};

class KeyPressEvent : public Event {
public:
  KeyPressEvent() = default;
  KeyPressEvent(KeyPressEvent const&) = default;
  KeyPressEvent(KeyPressEvent&&) noexcept = default;
  KeyPressEvent(char key, int sleepTime) : _key {key}, _sleepTime {sleepTime} {}

  auto solve(Display* display, Window window) -> void override {
    KeyActionEvent(_key, _sleepTime / 2, true).solve(display, window);
    KeyActionEvent(_key, _sleepTime / 2, false).solve(display, window);
  }

private:
  char _key {};
  int _sleepTime {};
};

class KeyCombinationEvent : public KeyPressEvent {
public:
  enum class Modifiers { SHIFT, CTRL };
  KeyCombinationEvent() = default;
  KeyCombinationEvent(KeyCombinationEvent const&) = default;
  KeyCombinationEvent(KeyCombinationEvent&&) noexcept = default;
  KeyCombinationEvent(char key, int sleepTime, Modifiers modifier) :
      KeyPressEvent {key, sleepTime}, _modifier {modifier} {}

  auto solve(Display* display, Window window) -> void override {
    KeySym keySym;
    switch (_modifier) {
      using enum Modifiers;
      case SHIFT: {
        keySym = XKeysymToKeycode(display, XK_Shift_L);
        break;
      }
      case CTRL: {
        keySym = XKeysymToKeycode(display, XK_Control_L);
        break;
      }
    }
    XTestFakeKeyEvent(display, keySym, True, CurrentTime);
    XSync(display, False);
    KeyPressEvent::solve(display, window);
    XTestFakeKeyEvent(display, keySym, False, CurrentTime);
    XSync(display, False);
  }

private:
  Modifiers _modifier {};
};

class CommandEvent : public Event {
public:
  CommandEvent() = default;
  CommandEvent(CommandEvent const&) = default;
  CommandEvent(CommandEvent&&) noexcept = default;
  explicit CommandEvent(std::string const& _other) : _command {_other} {}

  auto solve(Display* display, Window window) -> void override {
    KeyPressEvent('~', sleepTime).solve(display, window);
    for (auto const& c : _command) {
      if (c == '_') {
        KeyCombinationEvent(c, sleepTime, KeyCombinationEvent::Modifiers::SHIFT).solve(display, window);
      } else {
        KeyPressEvent(c, sleepTime).solve(display, window);
      }
    }
    KeyPressEvent(10, sleepTime).solve(display, window);
    KeyPressEvent(27, sleepTime).solve(display, window);
  }

private:
  static constexpr auto sleepTime = 7500;
  std::string _command {};
};

class RotationEvent : public Event {
public:
  enum class Axis { OX, OY };

  RotationEvent() = default;
  RotationEvent(RotationEvent const&) = default;
  RotationEvent(RotationEvent&&) noexcept = default;
  RotationEvent(float angle, Axis axis) : _angle {angle}, _axis {axis} {}

  auto solve(Display* display, Window window) -> void override {
    if (_axis == Axis::OX) {
      FullStrafeEvent(Point {static_cast<int>(_angle * degreeToPixelRatio), 0}, 20, 2, sleepTime)
          .solve(display, window);
    } else {
      FullStrafeEvent(Point {0, static_cast<int>(_angle * degreeToPixelRatio)}, 20, 2, sleepTime)
          .solve(display, window);
    }
  }

private:
  static constexpr auto degreeToPixelRatio = 26.6667f;
  static constexpr auto sleepTime = 5000;
  float _angle;
  Axis _axis;
};

class Movement {
public:
  Movement() = default;
  Movement(Movement const&) = default;
  Movement(Movement&&) noexcept = default;
  explicit Movement(Vector const& movement) : _movementVector {movement} {}

protected:
  auto getKeys() -> std::vector<char> {
    std::vector<char> inputs {};
    if (std::abs(_movementVector.x) >= errorMargin) {
      inputs.push_back(_movementVector.x > 0 ? 'w' : 's');
    }
    if (std::abs(_movementVector.y) >= errorMargin) {
      inputs.push_back(_movementVector.y > 0 ? 'a' : 'd');
    }
    return inputs;
  }
  Vector _movementVector {};

private:
  static constexpr auto errorMargin = 1.0f;
};

class MovementEvent : public Movement, public Event {
public:
  MovementEvent() = default;
  MovementEvent(MovementEvent const&) = default;
  MovementEvent(MovementEvent&&) noexcept = default;
  MovementEvent(Vector const& vector, int duration) : Movement(vector), _duration {duration} {}

  auto solve(Display* display, Window window) -> void override {
    auto inputs = getKeys();
    for (auto const& input : inputs) {
      KeyActionEvent(input, 50, true).solve(display, window);
    }
    usleep(_duration);
    for (auto const& input : inputs) {
      KeyActionEvent(input, 50, false).solve(display, window);
    }
  }

private:
  int _duration {};
};

class JumpEvent : public Movement, public Event {
public:
  JumpEvent() = default;
  JumpEvent(JumpEvent const&) = default;
  JumpEvent(JumpEvent&&) noexcept = default;
  JumpEvent(Vector const& vector, bool crouched) : Movement {vector}, _crouched {crouched} {}

  auto solve(Display* display, Window window) -> void override {
    KeyPressEvent(' ', sleepTime).solve(display, window);
    auto inputs = getKeys();
    for (auto const& input : inputs) {
      if (_crouched) {
        KeyCombinationEvent(input, sleepTime, KeyCombinationEvent::Modifiers::CTRL).solve(display, input);
      } else {
        KeyPressEvent(input, sleepTime).solve(display, input);
      }
    }
  }

private:
  static constexpr auto sleepTime = 0;
  bool _crouched {false};
};
} // namespace gabe
