//
// Created by stefan on 6/5/24.
//

#pragma once

#include "server/Server.hpp"

namespace gabe {

class Integrator {
public:
  explicit Integrator(GameState& state) : server {state} {}
  auto run() -> std::jthread {
    auto mainLoop = [this]() {
      while (!_stopped) {
        server.getClient();
      }
    };
    return std::jthread {mainLoop};
  }

  auto stop() -> void { _stopped = true; }

private:
  server::Server server;
  bool volatile _stopped {false};
};
} // namespace gabe