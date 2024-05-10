//
// Created by stefan on 3/28/24.
//

#pragma once
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
static constexpr auto expectedScreenWidth = 1920;
static constexpr auto expectedScreenHeight = 1036;

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

class MouseMoveEvent : public Event {
public:
  MouseMoveEvent() = default;
  MouseMoveEvent(MouseMoveEvent const&) = default;
  MouseMoveEvent(MouseMoveEvent&&) noexcept = default;
  explicit MouseMoveEvent(Point const& point) : _point {point} {}

  auto solve(Display* display, Window window) -> void override {
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

  auto solve(Display* display, Window window) -> void override {
    XWindowAttributes attr;
    int revert;
    XGetInputFocus(display, &window, &revert);
    XGetWindowAttributes(display, window, &attr);

    XSelectInput(display, window, PointerMotionMask);
    XFlush(display);

    Point p {};
    for (auto idx = 0; idx < itCount; ++idx) {
      p = _totalMovement * idx / itCount;
      XSelectInput(display, window, PointerMotionMask);
      XFlush(display);

      XTestFakeMotionEvent(display, -1, p.x, p.y, CurrentTime);
      log(std::format("Moved mouse to x={}, y={}", p.x, p.y), OpState::SUCCESS);
      XFlush(display);
      usleep(sleepTime);
    }

    log(std::format("Treated strafe mouse event of distance x={}, y={}", _totalMovement.x, _totalMovement.y), OpState::INFO);
  }

private:
  static constexpr auto sleepTime = 3000;
  static constexpr auto itCount = 50;
  Point _totalMovement {};
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
    log("Treated screen capture event", OpState::INFO);
    XDestroyImage(img);
  }

private:
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
  }

  unsigned char* _pTargetData;
};

class MouseClickEvent : public Event {
public:
  enum class Button : int {
    LEFT_BUTTON = Button1, 
    RIGHT_BUTTON = Button2, 
    WHEEL_PRESSED = Button3, 
    WHEEL_UP = Button4, 
    WHEEL_DOWN = Button5 
  };

  MouseClickEvent() = default;
  MouseClickEvent(MouseClickEvent const&) = default;
  MouseClickEvent(MouseClickEvent&&) noexcept = default;
  explicit MouseClickEvent(Button val) : _buttonType {val} {}

  auto solve(Display* display, Window window) -> void override {
    auto toString = [this]() -> std::string {
      switch(_buttonType) {
        using enum Button;
        case LEFT_BUTTON: return "left click";
        case RIGHT_BUTTON: return "right click";
        case WHEEL_PRESSED: return "middle click";
        case WHEEL_UP: return "wheel up";
        case WHEEL_DOWN: return "wheel down";
      }
      assert(false && "Button type undefined");
      return "<undefined button type>";
    };

    auto buttonId = static_cast<std::underlying_type_t<Button>>(_buttonType);
    XTestFakeButtonEvent(display, buttonId, True, CurrentTime);
    usleep(sleepTime);
    XTestFakeButtonEvent(display, buttonId, False, CurrentTime);
    log("Treated mouse click event of type " + toString(), OpState::INFO);
  }

private:
  static constexpr auto sleepTime = 100;
  Button _buttonType {};
};

class ShootEvent : public Event {
public:
  ShootEvent() = default;
  ShootEvent(ShootEvent const&) = default;
  ShootEvent(ShootEvent&&) noexcept = default;
  explicit ShootEvent(Point const& movement) : _totalMovement {movement} {}

  auto solve(Display* display, Window window) -> void override {
    StrafeEvent(_totalMovement).solve(display, window);
    MouseClickEvent(MouseClickEvent::Button::LEFT_BUTTON).solve(display, window);
    log("Treated shoot event", OpState::INFO);
  }

private:
  Point _totalMovement {};
};

class KeyPressEvent : public Event {
public:
  KeyPressEvent() = default;
  KeyPressEvent(KeyPressEvent const&) = default;
  KeyPressEvent(KeyPressEvent&&) noexcept = default;
  explicit KeyPressEvent(char key) : _key {key} {}

  auto solve(Display* display, Window window) -> void override {
    KeySym keyCode = XKeysymToKeycode(display, _key);
    XTestFakeKeyEvent(display, keyCode, True, CurrentTime);
    XSync(display, False);
    usleep(sleepTime);
    XTestFakeKeyEvent(display, keyCode, False, CurrentTime);
    XSync(display, False);
    log(std::format("Treated key pres event of {}", _key), OpState::INFO);
  }

private:
  static constexpr auto sleepTime = 100;
  char _key {};
};
} // namespace gabe
