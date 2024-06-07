//
// Created by stefan on 6/7/24.
//

#pragma once

#include "JsonUpdatable.hpp"
#include "utils/logger/Logger.hpp"

namespace gabe {
class Round : public JsonUpdatable<Round> {
public:
  enum class Stage { NONE, START, LIVE, OVER };
  enum class BombState { NOT_PLANTED, PLANTED, DEFUSED };


  auto jsonUpdate(cds::json::JsonObject const& jsonObject) noexcept(false) -> void {
    using namespace cds;

    using enum Stage;
    using E = MapEntry<StringView, Stage>;
    static auto const stageMatcher = cds::mapOf(E {"freezetime", START}, E {"live", LIVE}, E {"over", OVER});

    using enum BombState;
    using F = MapEntry<StringView, BombState>;
    static auto const bombStateMatcher = cds::mapOf(F {"planted", PLANTED}, F {"defused", DEFUSED});

    try {
      auto const& round = jsonObject.getJson("round");
      _stage = stageMatcher[round.getString("phase")];
      try {
        _bombPlanted = bombStateMatcher[jsonObject.getString("bomb")];
      } catch (...) {
        //empty on purpose
      }
    } catch (KeyException const& e) {
      log("Exception encountered while updating round state " + e.toString(), OpState::FAILURE);
      throw e;
    }
  }

private:
  BombState _bombPlanted {BombState::NOT_PLANTED};
  Stage _stage {Stage::NONE};
};
} // namespace gabe