//
// Created by stefan on 3/28/24.
//

#pragma once
#include "Exceptions.hpp"
#include <X11/Xutil.h>
#include <cstring>
#include <jpeglib.h>
#include <types.hpp>
#include <unistd.h>
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
  auto operator*(int value) const -> Point { return {x * value, y * value}; }
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
      log(std::format("Moved mouse to x={}, y ={}", p.x, p.y), OpState::SUCCESS);
      XFlush(display);
      usleep(3000);
    }

    log(std::format("Treated strafe mouse event of distance x={}, y={}", _totalMovement.x, _totalMovement.y), OpState::INFO);
  }

private:
  static constexpr auto itCount = 50;
  Point _totalMovement {};
};

class ScreenshotEvent : public Event {
public:
  ScreenshotEvent() = default;
  ScreenshotEvent(ScreenshotEvent const&) = default;
  ScreenshotEvent(ScreenshotEvent&&) noexcept = default;

  auto solve(Display* display, Window window) noexcept(false) -> void override {
    XWindowAttributes attr;
    XGetWindowAttributes(display, window, &attr);

    XImage* img = XGetImage(display, window, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);
    char* data = img->data;
    auto* jpgData = new unsigned char[attr.width * attr.height * 3];
    int bytesPerPixel = img->bits_per_pixel / 8;
    int bytesPerLine = img->bytes_per_line;
    for (int y = 0; y < attr.height; ++y) {
      for (int x = 0; x < attr.width; ++x) {
        int offset = y * bytesPerLine + x * bytesPerPixel;
        int jpgOffset = y * attr.width * 3 + x * 3;

        jpgData[jpgOffset + 0] = data[offset + 2];
        jpgData[jpgOffset + 1] = data[offset + 1];
        jpgData[jpgOffset + 2] = data[offset + 0];
      }
    }
    saveImage("image.jpg", jpgData, attr.width, attr.height);
    delete[] jpgData;
    log(std::format("Treated screen capture event; width={} height={}", attr.width, attr.height), OpState::INFO);
    XDestroyImage(img);
  }

private:
  static auto saveImage(std::string const& filename, unsigned char* data, int width, int height) -> void {
    struct jpeg_compress_struct cinfo {};
    struct jpeg_error_mgr jerr {};

    FILE* outfile;
    JSAMPROW row_pointer[1];
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    outfile = fopen(filename.c_str(), "wb");

    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3; // RGB
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);

    jpeg_start_compress(&cinfo, TRUE);

    row_stride = width * 3; // RGB components per row

    while (cinfo.next_scanline < cinfo.image_height) {
      row_pointer[0] = &data[cinfo.next_scanline * row_stride];
      jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
  }
};
} // namespace gabe
