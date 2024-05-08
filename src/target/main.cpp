#include <engine/Engine.hpp>

namespace {
using namespace gabe;
}

int main() {
  WindowController wc {"../scripts/find_csXwindow.sh"};
  Engine engine {wc};
  return engine.run();
}
