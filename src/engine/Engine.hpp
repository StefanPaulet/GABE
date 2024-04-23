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


    _windowController.add_event(std::make_unique<MouseClickEvent>(MouseClickEvent::Button::LEFT_BUTTON));
    for (auto idx = 0; idx < 10; ++idx) {
      _windowController.add_event(std::make_unique<KeyPressEvent>('a' + idx));
      _windowController.add_event(std::make_unique<KeyPressEvent>('0' + idx));
    }
    _windowController.add_event(std::make_unique<MouseClickEvent>(MouseClickEvent::Button::RIGHT_BUTTON));
    sleep(5);
    _windowController.stop();
    thread.join();

    log("Stopped processing commands", OpState::SUCCESS);
  }

private:
  WindowController& _windowController;
};
} // namespace gabe