#include <engine/Engine.hpp>

namespace {
using namespace gabe;
using namespace gabe::utils;
using namespace gabe::path;
}

int main() {
  //  Engine engine {"../", "/home/stefan/.local/share/Steam/steamapps/common/Counter-Strike Global Offensive/"};
  Engine engine {"../", "/mnt/SSD-SATA/SteamLibrary/steamapps/common/Counter-Strike Global Offensive/"};
  return engine.run();
}
