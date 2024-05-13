#include <engine/Engine.hpp>

namespace {
using namespace gabe;
}

int main() {
  Engine engine {"../scripts"};
  return engine.run();
}
