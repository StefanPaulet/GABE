#include <engine/Engine.hpp>

namespace {
using namespace gabe;
using namespace gabe::utils;
}

int main() {
  Engine engine {"../", "/mnt/SSD-SATA/SteamLibrary/steamapps/common/Counter-Strike Global Offensive/"};
  return engine.run();
}
