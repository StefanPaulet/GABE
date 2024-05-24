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
    A_SITE,
    TOP_MID,
    T_SPAWN_TO_MID,
    BUNELU,
    MID_TO_SHORT,
    T_TO_SHORT,
    SHORT_CORRIDOR,
    SHORT_ABOVE_CT,
    SHORT_TO_A
  };
  enum class RequiredMovement { NONE, JUMP, JUMP_AND_CROUCH, RUN_AND_JUMP, GO_TO_AND_JUMP };
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
        case A_SITE: {
          return "A_SITE";
        }
        case TOP_MID: {
          return "TOP_MID";
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

private:
  auto buildZones() -> void {
    using enum ZoneName;
    NamedZone tSpawn {Zone {Position {-1176.63f, -665.08f, 187.94f}, Position {341.33f, -999.97f, 65.68f}}, T_SPAWN};
    tSpawn.zone.obstacles.emplace_back(Position {-878.56f, -722.03f, 186.22f}, Position {-982.03f, -638.03f, 192.16f});
    _zones.push_back(tSpawn);

    NamedZone tSpawnToLong {Zone {Position {139.25f, -384.96f, 65.6f}, Position {747.97f, 235.97f, 72.28f}},
                            T_SPAWN_TO_LONG};
    _zones.push_back(tSpawnToLong);

    NamedZone tDoors {Zone {Position {583.92f, 255.87f, 63.74f}, Position {740.4f, 352.61f, 64.43f}}, T_DOORS};
    _zones.push_back(tDoors);

    NamedZone doorsCorridor {Zone {Position {593.03f, 350.26f, 65.47f}, Position {540.0f, 583.19f, 65.35f}},
                             DOORS_CORRIDOR};
    _zones.push_back(doorsCorridor);

    NamedZone longDoors {Zone {Position {593.03f, 620.24f, 65.34f}, Position {718.51f, 804.55f, 64.02f}}, LONG_DOORS};
    _zones.push_back(longDoors);

    NamedZone outsideDoorsLong {Zone {Position {516.03f, 808.03f, 65.79f}, Position {1232.43f, 1195.97f, 64.02f}},
                                OUTSIDE_DOORS_LONG};
    _zones.push_back(outsideDoorsLong);

    NamedZone nearDoorsLong {Zone {Position {968.03f, 215.03f, 75.7f}, Position {1227.96f, 769.19f, 71.39f}},
                             NEAR_DOORS_LONG};
    _zones.push_back(nearDoorsLong);

    NamedZone pit {Zone {Position {1292.03f, 741.21f, 48.86f}, Position {1571.97f, 201.03f, -117.03f}}, PIT};
    _zones.push_back(pit);

    NamedZone farLong {Zone {Position {1271.77f, 804.34f, 60.56f}, Position {1576.3f, 1594.73f, 64.05f}}, FAR_LONG};
    _zones.push_back(farLong);

    NamedZone aSiteLong {Zone {Position {1289.37f, 1691.23f, 64.13f}, Position {1601.38f, 2306.24f, 68.40f}},
                         A_SITE_LONG};
    _zones.push_back(aSiteLong);

    NamedZone ramp {Zone {Position {1601.38f, 2306.24f, 68.40f}, Position {1311.11f, 2772.2f, 179.13f}}, RAMP};
    _zones.push_back(ramp);

    NamedZone aSite {Zone {Position {1235.97f, 2348.03f, 163.02f}, Position {1051.03f, 3059.97f, 194.33f}}, A_SITE};
    aSite.zone.obstacles.emplace_back(Position {1264.87f, 2460.97f, 191.03f}, Position {1176.88f, 2561.03f, 159.96f});
    aSite.zone.obstacles.emplace_back(Position {1097.64f, 2575.82f, 160.12f}, Position {989.21f, 2411.97f, 191.09f});
    _zones.push_back(aSite);

    NamedZone topMid {Zone {Position {-361.07f, -593.97f, 64.98f}, Position {-491.97f, 189.88f, 65.03f}}, TOP_MID};
    topMid.zone.obstacles.emplace_back(Position {-425.54f, -43.97f, 62.81f}, Position {-491.97f, -228.03f, 640.06f});
    _zones.push_back(topMid);

    NamedZone tSpawnToMid {Zone {Position {-60.08f, -457.97f, 63.05f}, Position {447.79f, 265.12f, 65.53f}},
                           T_SPAWN_TO_MID};
    _zones.push_back(tSpawnToMid);

    NamedZone bunelu {Zone {Position {-511.01f, -212.03f, 66.21f}, Position {-621.97f, 627.97f, 72.09f}}, BUNELU};
    _zones.push_back(bunelu);

    NamedZone midToShort {Zone {Position {-493.30f, 746.29f, 64.00f}, Position {-145.65f, 308.03f, 64.74f}},
                          MID_TO_SHORT};
    midToShort.zone.obstacles.emplace_back(Position {-171.09f, 539.17f, 63.63f}, Position {-258.75f, 582.25f, 64.16f});
    _zones.push_back(midToShort);


    NamedZone tToShort {Zone {Position {-149.03f, 575.31f, 64.34f}, Position {-211.97f, 1516.97f, 64.54f}}, T_TO_SHORT};
    _zones.push_back(tToShort);

    NamedZone shortCorridor {Zone {Position {-50.15f, 1348.81f, 64.34f}, Position {273.04f, 1628.74f, 70.01f}},
                             SHORT_CORRIDOR};
    _zones.push_back(shortCorridor);


    NamedZone shortAboveCt {Zone {Position {273.06f, 1631.88f, 70.37f}, Position {503.72f, 2444.03f, 159.83f}},
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
    addTransition(T_SPAWN, T_SPAWN_TO_LONG);
    addTransition(T_SPAWN, TOP_MID, JUMP, true);

    addTransition(T_SPAWN_TO_LONG, T_DOORS);
    addTransition(T_SPAWN_TO_LONG, T_SPAWN_TO_MID);

    addTransition(T_DOORS, DOORS_CORRIDOR);

    addTransition(DOORS_CORRIDOR, LONG_DOORS);

    addTransition(LONG_DOORS, OUTSIDE_DOORS_LONG);

    addTransition(OUTSIDE_DOORS_LONG, NEAR_DOORS_LONG);
    addTransition(OUTSIDE_DOORS_LONG, FAR_LONG);

    addTransition(NEAR_DOORS_LONG, PIT, JUMP, true);

    addTransition(FAR_LONG, A_SITE_LONG);
    addTransition(FAR_LONG, PIT);

    addTransition(A_SITE_LONG, RAMP);

    addTransition(RAMP, A_SITE, GO_TO_AND_JUMP, true);

    addTransition(A_SITE, RAMP, JUMP, true);
    addTransition(A_SITE, RAMP);

    addTransition(TOP_MID, BUNELU);

    addTransition(BUNELU, T_SPAWN_TO_MID);
    addTransition(BUNELU, MID_TO_SHORT);

    addTransition(T_SPAWN_TO_MID, MID_TO_SHORT);

    addTransition(MID_TO_SHORT, T_TO_SHORT);

    addTransition(T_TO_SHORT, SHORT_CORRIDOR);

    addTransition(SHORT_CORRIDOR, SHORT_ABOVE_CT);

    addTransition(SHORT_ABOVE_CT, SHORT_TO_A);

    addTransition(SHORT_TO_A, A_SITE);
  }

  auto addTransition(ZoneName start, ZoneName target, RequiredMovement movement = RequiredMovement::NONE,
                     bool oneWay = false) -> void {
    auto zoneSelector = [](NamedZone const& z) { return z.name; };
    auto startZone = std::ranges::find(_zones.begin(), _zones.end(), start, zoneSelector);
    auto endZone = std::ranges::find(_zones.begin(), _zones.end(), target, zoneSelector);

    _transitions[startZone->zone].emplace_back(endZone->zone, movement);
    if (!oneWay) {
      _transitions[endZone->zone].emplace_back(startZone->zone, movement);
    }
  }

  std::map<Zone, std::vector<Transition>> _transitions;
  std::vector<NamedZone> _zones;
};

} // namespace gabe
