#include <array>
#include <engine/Engine.hpp>
#include <iostream>
#include <memory>
#include <windowController/WindowController.hpp>

namespace {
using namespace gabe;
}

int main() {
  gabe::WindowController wc {"../scripts/find_csXwindow.sh"};
  Engine engine {wc};
  return engine.run();
}