//
// Created by stefan on 6/6/24.
//

#pragma once

#include "JsonUpdatable.hpp"
#include "utils/logger/Logger.hpp"

namespace gabe {
class Player : public JsonUpdatable<Player> {
public:
  auto jsonUpdate(cds::json::JsonObject const& jsonObject) -> void {
    using namespace cds;
    try {
      auto const& playerState = jsonObject.getJson("player").getJson("state");
      _health = playerState.getInt("health");
      _money = playerState.getInt("money");
      _armor = playerState.getInt("armor");
      _helmet = playerState.getInt("helmet");
      _flashed = playerState.getInt("flashed");
    } catch (Exception const& e) {
      log("Exception encountered while updating player state " + e.toString(), OpState::FAILURE);
    }
  }

  [[nodiscard]] auto health() const -> int { return _health; }
  [[nodiscard]] auto money() const -> int { return _money; }
  [[nodiscard]] auto armor() const -> int { return _armor; }
  [[nodiscard]] auto hasHelmet() const -> bool { return _helmet; }
  [[nodiscard]] auto isFlashed() const -> bool { return _flashed; }

private:
  int _health {};
  int _money {};
  int _armor {};
  bool _helmet {};
  bool _flashed {};
};
} // namespace gabe
