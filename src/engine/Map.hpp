//
// Created by stefan on 5/13/24.f
//

#pragma once

#include <algorithm>
#include <map>
#include <types.hpp>
#include <vector>

namespace gabe {

struct Zone {
  Volume volume;
  std::vector<Volume> obstacles {};

  auto operator<=>(Zone const& other) const { return volume.operator<=>(other.volume); }
  auto operator==(Zone const& other) const -> bool { return volume == other.volume; }
};

class Map {
public:
  enum class ZoneName {
    NO_ZONE,
    T_SPAWN,
    T_SPAWN_EXIT,
    T_SPAWN_TO_LONG,
    T_DOORS,
    DOORS_CORRIDOR,
    LONG_DOORS,
    OUTSIDE_DOORS_LONG,
    NEAR_DOORS_LONG,
    PIT,
    FAR_LONG,
    A_SITE_LONG,
    RAMP,
    TOP_OF_RAMP,
    GOOSE,
    A_SITE,
    TOP_MID,
    OUTSIDE_TOP_MID,
    T_SPAWN_TO_MID,
    BUNELU,
    MID_TO_SHORT,
    T_TO_SHORT,
    SHORT_CORRIDOR,
    SHORT_STAIRS,
    SHORT_ABOVE_CT,
    SHORT_TO_A
  };
  enum class RequiredMovement { NONE, JUMP, JUMP_AND_CROUCH, RUN_AND_JUMP };
  struct NamedZone {
    Zone zone;
    ZoneName name;

    auto operator==(NamedZone const& other) const -> bool { return name == other.name; }
    auto operator!=(NamedZone const& other) const -> bool { return name != other.name; }

    auto toString() const -> std::string {
      switch (name) {
        using enum Map::ZoneName;
        case T_SPAWN: {
          return "T_SPAWN";
        }
        case T_SPAWN_EXIT: {
          return "T_SPAWN_EXIT";
        }
        case T_SPAWN_TO_LONG: {
          return "T_SPAWN_TO_LONG";
        }
        case T_DOORS: {
          return "T_DOORS";
        }
        case DOORS_CORRIDOR: {
          return "DOORS_CORRIDOR";
        }
        case LONG_DOORS: {
          return "LONG_DOORS";
        }
        case OUTSIDE_DOORS_LONG: {
          return "OUTSIDE_DOORS_LONG";
        }
        case NEAR_DOORS_LONG: {
          return "NEAR_DOORS_LONG";
        }
        case PIT: {
          return "PIT";
        }
        case FAR_LONG: {
          return "FAR_LONG";
        }
        case A_SITE_LONG: {
          return "A_SITE_LONG";
        }
        case RAMP: {
          return "RAMP";
        }
        case TOP_OF_RAMP: {
          return "TOP_OF_RAMP";
        }
        case GOOSE: {
          return "GOOSE";
        }
        case A_SITE: {
          return "A_SITE";
        }
        case TOP_MID: {
          return "TOP_MID";
        }
        case OUTSIDE_TOP_MID: {
          return "OUTSIDE_TOP_MID";
        }
        case T_SPAWN_TO_MID: {
          return "T_SPAWN_TO_MID";
        }
        case BUNELU: {
          return "BUNELU";
        }
        case MID_TO_SHORT: {
          return "MID_TO_SHORT";
        }
        case T_TO_SHORT: {
          return "T_TO_SHORT";
        }
        case SHORT_CORRIDOR: {
          return "SHORT_CORRIDOR";
        }
        case SHORT_STAIRS: {
          return "SHORT_STAIRS";
        }
        case SHORT_ABOVE_CT: {
          return "SHORT_ABOVE_CT";
        }
        case SHORT_TO_A: {
          return "SHORT_TO_A";
        }
        default: {
          return "NO ZONE";
        }
      }
    }
  };
  struct Transition {
    Zone zone;
    RequiredMovement movement;
    Volume transitionArea {};
  };

  Map() {
    buildZones();
    buildZoneTransitions();
  }

  auto findZone(Position const& position) const -> NamedZone {
    auto zoneCompare = [position](NamedZone const& z1, NamedZone const& z2) {
      return z1.zone.volume.distance(position) < z2.zone.volume.distance(position);
    };
    return *std::ranges::min_element(_zones.begin(), _zones.end(), zoneCompare);
  }

  auto getZoneByName(ZoneName zoneName) const -> NamedZone {
    return *std::ranges::find(_zones.begin(), _zones.end(), zoneName, [](NamedZone const& z) { return z.name; });
  }

  auto getZoneByVolume(Zone const& zone) const -> NamedZone {
    return *std::ranges::find(_zones.begin(), _zones.end(), zone, [](NamedZone const& z) { return z.zone; });
  }

  [[nodiscard]] auto const& transitions() const { return _transitions; }

  auto possibleTransitions(Zone const& startZone, Zone const& targetZone) const -> std::vector<Transition> {
    std::vector<Transition> result {};
    for (auto const& transition : _transitions.at(startZone)) {
      if (transition.zone == targetZone) {
        result.push_back(transition);
      }
    }
    return result;
  }

private:
  auto buildZones() -> void {
    using enum ZoneName;
    NamedZone tSpawn {Zone {Position {-1176.63f, -665.08f, 187.94f}, Position {439.72f, -999.97f, 67.21f}}, T_SPAWN};
    tSpawn.zone.obstacles.emplace_back(Position {-878.56f, -722.03f, 186.22f}, Position {-982.03f, -638.03f, 192.16f});
    _zones.push_back(tSpawn);

    NamedZone tSpawnExit {Zone {Position {375.97f, -481.48f, 63.87f}, Position {-491.97f, -634.46f, 221.34f}},
                          T_SPAWN_EXIT};
    _zones.push_back(tSpawnExit);

    NamedZone tSpawnToLong {Zone {Position {111.07f, -481.46f, 65.3f}, Position {747.97f, 235.97f, 72.28f}},
                            T_SPAWN_TO_LONG};
    tSpawnToLong.zone.obstacles.emplace_back(Position {375.97f, -499.79f, 63.87f},
                                             Position {572.03f, -370.01f, 74.45f});
    tSpawnToLong.zone.obstacles.emplace_back(Position {148.03f, -182.03f, 71.87f}, Position {211.8f, 19.62f, 69.4f});
    tSpawnToLong.zone.obstacles.emplace_back(Position {124.08f, 101.10f, 73.1f}, Position {170.13f, 39.09f, 71.09f});
    tSpawnToLong.zone.obstacles.emplace_back(Position {734.93f, -168.89f, 71.0f}, Position {598.21f, 51.49f, 60.32f});
    _zones.push_back(tSpawnToLong);

    NamedZone tDoors {Zone {Position {584.03f, 261.05f, 63.74f}, Position {700.69f, 339.08f, 64.48f}}, T_DOORS};
    _zones.push_back(tDoors);

    NamedZone doorsCorridor {Zone {Position {539.03f, 346.59f, 65.31f}, Position {733.76f, 707.08f, 71.95f}},
                             DOORS_CORRIDOR};
    doorsCorridor.zone.obstacles.emplace_back(Position {733.76f, 707.08f, 71.95f}, Position {653.11f, 574.38f, 64.6f});
    _zones.push_back(doorsCorridor);

    NamedZone longDoors {Zone {Position {568.17f, 693.9f, 64.99f}, Position {708.52f, 794.57f, 63.9f}}, LONG_DOORS};
    _zones.push_back(longDoors);

    NamedZone outsideDoorsLong {Zone {Position {516.03f, 808.03f, 65.79f}, Position {1247.34f, 1195.97f, 64.02f}},
                                OUTSIDE_DOORS_LONG};
    outsideDoorsLong.zone.obstacles.emplace_back(Position {700.11f, 1081.83f, 64.61f},
                                                 Position {925.3f, 1214.82f, 100.03f});
    outsideDoorsLong.zone.obstacles.emplace_back(Position {925.3f, 1214.82f, 100.03f},
                                                 Position {984.85f, 1123.97f, 64.55f});
    _zones.push_back(outsideDoorsLong);

    NamedZone nearDoorsLong {Zone {Position {968.03f, 215.03f, 75.7f}, Position {1227.96f, 769.19f, 71.39f}},
                             NEAR_DOORS_LONG};
    _zones.push_back(nearDoorsLong);

    NamedZone pit {Zone {Position {1292.03f, 741.21f, 48.86f}, Position {1571.97f, 201.03f, -117.03f}}, PIT};
    _zones.push_back(pit);

    NamedZone farLong {Zone {Position {1271.77f, 804.34f, 60.56f}, Position {1593.97f, 1662.56f, 63.64f}}, FAR_LONG};
    _zones.push_back(farLong);

    NamedZone aSiteLong {Zone {Position {1593.97f, 1662.56f, 63.64f}, Position {1300.03f, 2302.24f, 77.80f}},
                         A_SITE_LONG};
    _zones.push_back(aSiteLong);

    NamedZone ramp {Zone {Position {1601.38f, 2306.24f, 68.40f}, Position {1311.11f, 2772.2f, 179.13f}}, RAMP};
    _zones.push_back(ramp);

    NamedZone topOfRamp {Zone {Position {1561.97f, 3059.97f, 190.74f}, Position {1311.11f, 2772.2f, 179.13f}},
                         TOP_OF_RAMP};
    _zones.push_back(topOfRamp);

    NamedZone goose {Zone {Position {1235.97f, 2632.93f, 190.36f}, Position {1051.03f, 3059.86f, 193.89f}}, GOOSE};
    _zones.push_back(goose);

    NamedZone aSite {Zone {Position {1249.08f, 2616.88f, 190.6f}, Position {1056.03f, 2347.07f, 190.53f}}, A_SITE};
    aSite.zone.obstacles.emplace_back(Position {1265.55f, 2561.03f, 190.87f}, Position {1176.88f, 2460.97f, 159.96f});
    aSite.zone.obstacles.emplace_back(Position {1097.64f, 2575.82f, 160.12f}, Position {989.21f, 2411.97f, 191.09f});
    _zones.push_back(aSite);

    NamedZone topMid {Zone {Position {-391.77f, -444.87f, 64.98f}, Position {-491.97f, 189.88f, 65.03f}}, TOP_MID};
    topMid.zone.obstacles.emplace_back(Position {-425.54f, -43.97f, 62.81f}, Position {-491.97f, -228.03f, 640.06f});
    _zones.push_back(topMid);

    NamedZone tSpawnToMid {Zone {Position {-60.08f, 457.97f, 63.05f}, Position {447.79f, 260.12f, 65.53f}},
                           T_SPAWN_TO_MID};
    _zones.push_back(tSpawnToMid);

    NamedZone bunelu {Zone {Position {-511.01f, 212.03f, 66.21f}, Position {-621.97f, 627.97f, 72.09f}}, BUNELU};
    _zones.push_back(bunelu);

    NamedZone outsideTopMid {Zone {Position {-77.02f, 308.03f, 64.33f}, Position {-493.39f, 553.24f, 63.69f}},
                             OUTSIDE_TOP_MID};
    _zones.push_back(outsideTopMid);

    NamedZone midToShort {Zone {Position {-493.39f, 553.24f, 63.69f}, Position {-149.03f, 751.5f, 65.77f}},
                          MID_TO_SHORT};
    midToShort.zone.obstacles.emplace_back(Position {-171.09f, 539.17f, 63.63f}, Position {-258.75f, 582.25f, 64.16f});
    _zones.push_back(midToShort);


    NamedZone tToShort {Zone {Position {-149.03f, 767.47f, 66.06f}, Position {-211.97f, 1516.97f, 64.54f}}, T_TO_SHORT};
    _zones.push_back(tToShort);

    NamedZone shortCorridor {Zone {Position {489.97f, 1539.69f, 64.58f}, Position {-132.02f, 1361.89f, 63.67f}},
                             SHORT_CORRIDOR};
    _zones.push_back(shortCorridor);

    NamedZone shortStairs {Zone {Position {273.03f, 1539.66f, 66.12f}, Position {489.97f, 1943.97f, 159.87f}},
                           SHORT_STAIRS};
    _zones.push_back(shortStairs);


    NamedZone shortAboveCt {Zone {Position {305.82f, 1970.14f, 162.21f}, Position {503.72f, 2444.03f, 159.83f}},
                            SHORT_ABOVE_CT};
    shortAboveCt.zone.obstacles.emplace_back(Position {411.97f, 1952.03f, 160.0f}, Position {494.18f, 2052.0f, 160.0f});
    _zones.push_back(shortAboveCt);


    NamedZone shortToA {Zone {Position {278.03f, 2437.36f, 161.86f}, Position {982.2f, 2762.22f, 163.58f}}, SHORT_TO_A};
    shortToA.zone.obstacles.emplace_back(Position {563.97f, 2697.82f, 160.4f}, Position {752.03f, 2763.97f, 161.62f});
    shortToA.zone.obstacles.emplace_back(Position {833.73f, 2676.38f, 159.51f}, Position {904.25f, 2763.97f, 163.73f});
    _zones.push_back(shortToA);

    std::ranges::for_each(_zones, [this](NamedZone const& zone) { _transitions[zone.zone] = {}; });
  }

  auto buildZoneTransitions() -> void {
    using enum ZoneName;
    using enum RequiredMovement;
    addTransition(T_SPAWN, T_SPAWN_EXIT, JUMP, true, {{-507.37f, -661.95f, 184.66f}, {-1.93f, -660.03f, 69.35f}});
    addTransition(T_SPAWN, T_SPAWN_EXIT, NONE, false, {{22.8f, -660.37f, 68.14f}, {321.43f, -665.32f, 62.87f}});

    addTransition(T_SPAWN_EXIT, T_SPAWN_TO_LONG);
    addTransition(T_SPAWN_EXIT, TOP_MID);

    addTransition(T_SPAWN_TO_LONG, T_DOORS, NONE, false, {{576.74f, 248.7f, 63.1f}, {622.43f, 249.67f, 63.33f}});
    addTransition(T_SPAWN_TO_LONG, T_SPAWN_TO_MID);

    addTransition(T_DOORS, DOORS_CORRIDOR);

    addTransition(DOORS_CORRIDOR, LONG_DOORS, NONE, false, {{629.97f, 705.0f, 64.65f}, {566.55f, 702.553f, 64.99f}});

    addTransition(LONG_DOORS, OUTSIDE_DOORS_LONG, NONE, false, {{650.03f, 790.3f, 64.26f}, {698.7f, 784.7f, 64.06f}});

    addTransition(OUTSIDE_DOORS_LONG, NEAR_DOORS_LONG);
    addTransition(OUTSIDE_DOORS_LONG, FAR_LONG);

    addTransition(NEAR_DOORS_LONG, PIT, JUMP, true, {{1227.97f, 342.84f, 72.75f}, {1227.97f, 743.17f, 71.3f}});

    addTransition(FAR_LONG, A_SITE_LONG);
    addTransition(FAR_LONG, PIT);

    addTransition(A_SITE_LONG, RAMP);

    addTransition(RAMP, A_SITE, JUMP_AND_CROUCH, true, {{1300.0f, 2481.87f, 128.46f}, {1300.0f, 2665.0f, 161.46f}});
    addTransition(RAMP, A_SITE, NONE, true, {{1297.82f, 2802.3f, 183.72f}, {1305.22f, 2711.42f, 170.18f}});
    addTransition(RAMP, TOP_OF_RAMP);

    addTransition(TOP_OF_RAMP, GOOSE);

    addTransition(GOOSE, A_SITE);

    addTransition(A_SITE, RAMP, JUMP, true, {{1235.9f, 2460.97f, 160.79f}, {1235.97f, 2348.03f, 162.14f}});

    addTransition(TOP_MID, BUNELU);
    addTransition(TOP_MID, OUTSIDE_TOP_MID);

    addTransition(OUTSIDE_TOP_MID, MID_TO_SHORT);
    addTransition(OUTSIDE_TOP_MID, BUNELU);

    addTransition(T_SPAWN_TO_MID, OUTSIDE_TOP_MID);

    addTransition(MID_TO_SHORT, T_TO_SHORT);

    addTransition(T_TO_SHORT, SHORT_CORRIDOR);

    addTransition(SHORT_CORRIDOR, SHORT_STAIRS, NONE, true, {{273.03f, 1625.75f, 67.71f}, {404.02f, 1622.72f, 68.89f}});

    addTransition(SHORT_STAIRS, SHORT_ABOVE_CT);

    addTransition(SHORT_ABOVE_CT, SHORT_TO_A);

    addTransition(SHORT_TO_A, A_SITE);
  }

  auto addTransition(ZoneName start, ZoneName target, RequiredMovement movement = RequiredMovement::NONE,
                     bool oneWay = false, Volume&& transitionArea = {}) -> void {
    auto zoneSelector = [](NamedZone const& z) { return z.name; };
    auto startZone = std::ranges::find(_zones.begin(), _zones.end(), start, zoneSelector);
    auto endZone = std::ranges::find(_zones.begin(), _zones.end(), target, zoneSelector);

    _transitions[startZone->zone].emplace_back(endZone->zone, movement, transitionArea);
    if (!oneWay) {
      _transitions[endZone->zone].emplace_back(startZone->zone, movement, transitionArea);
    }
  }

  std::map<Zone, std::vector<Transition>> _transitions;
  std::vector<NamedZone> _zones;
};

} // namespace gabe
