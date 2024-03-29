#include <array>
#include <iostream>
#include <memory>
#include <windowController/WindowController.hpp>

namespace {
using namespace gabe;
}

int main() {
  gabe::WindowController wc {"../scripts/find_csXwindow.sh"};
  std::unique_ptr<Event> mouseMoveEvent = std::make_unique<MouseMoveEvent>(Point {15, 15});
  wc.add_event(std::move(mouseMoveEvent));
  auto mainThread = wc.run();
  for (int i = 0; i < 255; ++i) {
    wc.add_event(std::make_unique<MouseMoveEvent>(Point {i * 10, i + 15}));
  }
  return 0;
}
