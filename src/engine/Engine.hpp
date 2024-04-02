//
// Created by stefan on 4/2/24.
//

#pragma once
#include "windowController/WindowController.hpp"

namespace gabe {
class Engine {
public:
  Engine() = delete;
  Engine(Engine const&) = delete;
  Engine(Engine&&) noexcept = delete;

  explicit Engine(WindowController& windowController) : _windowController {windowController} {}

  auto run() {
    log("Started processing commands", OpState::SUCCESS);
    auto thread = _windowController.run();

    for (int i = 0; i < 10; ++i) {
      auto sign = i % 2 ? -1 : 1;
      _windowController.add_event(std::make_unique<StrafeEvent>(Point {100 * sign, 0}));
      sleep(1);
    }
    sleep(5);
    _windowController.stop();
    thread.join();

    log("Stopped processing commands", OpState::SUCCESS);
  }

private:
  WindowController& _windowController;
};
} // namespace gabe