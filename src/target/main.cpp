#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>

void mouseClick(int button) {
  Display* display = XOpenDisplay(NULL);

  XEvent event;

  if (display == NULL) {
    fprintf(stderr, "Cannot initialize the display\n");
    exit(EXIT_FAILURE);
  }

  memset(&event, 0x00, sizeof(event));

  event.type = ButtonPress;
  event.xbutton.button = button;
  event.xbutton.same_screen = True;

  XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window,
                &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);

  event.xbutton.subwindow = event.xbutton.window;

  while (event.xbutton.subwindow) {
    event.xbutton.window = event.xbutton.subwindow;

    XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root,
                  &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
  }

  if (XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");

  XFlush(display);

  usleep(100000);

  event.type = ButtonRelease;
  event.xbutton.state = 0x100;

  if (XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");

  XFlush(display);

  XCloseDisplay(display);
}
auto click(long windowId) -> int {
  Display* display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Unable to open display\n");
    return 1;
  }

  std::cout << "Window id in func: " << windowId << '\n';
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


std::string exec(char const* cmd) {
  std::array<char, 128> buffer {};
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe {popen(cmd, "r"), &pclose};
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += std::string {buffer.data()};
  }
  return result;
}

int main() {
  try {
    // Execute your shell script
    std::string output = exec("../scripts/find_csXwindow.sh");
    std::cout << "Output of the script: " << output << '\n';
    auto windowID = std::stoi(output, nullptr, 16);
    std::cout << "Window id: " << windowID << '\n';

    click(windowID);
  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
