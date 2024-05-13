//
// Created by stefan on 5/10/24.
//
#pragma once

#include <condition_variable>
#include <utils/logger/Logger.hpp>

namespace gabe {
class Synchronizer {
public:
  Synchronizer() = default;
  Synchronizer(Synchronizer const&) = delete;
  Synchronizer(Synchronizer&&) noexcept = delete;

  auto requestSynchronization() -> void {
    std::unique_lock lockGuard {mutex};
    synchronize = true;
    conditionVariable.wait(lockGuard, [this] { return !synchronize; });
  }

  auto handleSynchronization() -> void {
    std::unique_lock lockGuard {mutex};
    synchronize = false;
    conditionVariable.notify_all();
  }

  [[nodiscard]] auto synchronizationRequired() const -> bool { return synchronize; }

private:
  bool synchronize {false};
  std::mutex mutex {};
  std::condition_variable conditionVariable {};
};
} // namespace gabe