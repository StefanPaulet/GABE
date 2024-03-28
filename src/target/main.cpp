#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <windowController/WindowController.hpp>

auto click(long windowId) -> int {
  Display* display = XOpenDisplay(nullptr);
  if (display == nullptr) {
    fprintf(stderr, "Unable to open display\n");
    return 1;
  }
  Window targetWindow = windowId; // Replace with actual window ID
  int x = 50;                     // Example X coordinate within the window
  int y = 100;                    // Example Y coordinate within the window

  // Set input focus to the target window
  for (int i = 0; i < 100; ++i) {
    XSetInputFocus(display, targetWindow, RevertToParent, CurrentTime);
    XFlush(display); // Flush the output buffer

    // Move the mouse cursor to the desired coordinates
    XTestFakeMotionEvent(display, -1, x, y, CurrentTime);
    XFlush(display); // Flush the output buffer

    // Simulate a left mouse button press and release
    XTestFakeButtonEvent(display, 1, True, CurrentTime);
    XTestFakeButtonEvent(display, 1, False, CurrentTime);
    XFlush(display); // Flush the output buffer
  }
  XCloseDisplay(display);
  return 0;
}

int main() {
  gabe::WindowController wc {};
  return 0;
}
