#include <engine/Engine.hpp>

namespace {
using namespace gabe;
}

int main() {
  Engine engine {"../scripts/find_csXwindow.sh"};
  return engine.run();
}
