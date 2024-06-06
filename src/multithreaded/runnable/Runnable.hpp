//
// Created by stefan on 6/6/24.
//

#pragma once

#include <thread>

namespace gabe {

template <typename D> class Runnable {
public:
  auto run() {
    auto endlessLoop = [this]() {
      while (!_stopped) {
        static_cast<D*>(this)->mainLoop();
      }
    };

    _thread = std::jthread {endlessLoop};
  }

  auto stop() -> void {
    _stopped = true;
    _thread.join();
  }

private:
  std::jthread _thread {};
  bool volatile _stopped {false};
};
} // namespace gabe