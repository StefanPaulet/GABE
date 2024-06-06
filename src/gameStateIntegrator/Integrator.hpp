//
// Created by stefan on 6/5/24.
//

#pragma once

#include "multithreaded/runnable/Runnable.hpp"
#include "server/Server.hpp"

namespace gabe {

class Integrator : public Runnable<Integrator> {
public:
  explicit Integrator(GameState& state) : server {state} {}
  auto mainLoop() -> void { server.getClient(); }

private:
  server::Server server;
};
} // namespace gabe