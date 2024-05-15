//
// Created by stefan on 5/13/24.f
//

#pragma once

#include <algorithm>
#include <map>
#include <types.hpp>
#include <vector>

namespace gabe {

struct MapZone {
  Volume volume;
  std::vector<Volume> obstacles {};
};

class Map {
public:
  enum class ZoneName {
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
    A_SITE
  };
  enum class RequiredMovement { NONE, JUMP, JUMP_AND_CROUCH, RUN_AND_JUMP, GO_TO_AND_JUMP };
  using Transition = std::pair<MapZone, RequiredMovement>;
  struct NamedZone {
    MapZone zone;
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
      }
    }
  };

  Map() {
    using enum ZoneName;
    NamedZone tSpawn {MapZone {Position {-1176.63f, -665.08f, 187.94f}, Position {341.33f, -999.97f, 65.68f}}, T_SPAWN};
    tSpawn.zone.obstacles.emplace_back(Position {-878.56f, -722.03f, 186.22f}, Position {-982.03f, -638.03f, 192.16f});
    _zones.push_back(tSpawn);

    NamedZone tSpawnToLong {MapZone {Position {130.77f, -420.88f, 65.6f}, Position {747.97f, 235.97f, 72.28f}},
                            T_SPAWN_TO_LONG};
    _zones.push_back(tSpawnToLong);

    NamedZone tDoors {MapZone {Position {583.92f, 255.87f, 63.74f}, Position {740.4f, 352.61f, 64.43f}}, T_DOORS};
    _zones.push_back(tDoors);

    NamedZone doorsCorridor {MapZone {Position {593.03f, 350.26f, 65.47f}, Position {540.0f, 583.19f, 65.35f}},
                             DOORS_CORRIDOR};
    _zones.push_back(doorsCorridor);

    NamedZone longDoors {MapZone {Position {593.03f, 620.24f, 65.34f}, Position {718.51f, 804.55f, 64.02f}},
                         LONG_DOORS};
    _zones.push_back(longDoors);

    NamedZone outsideDoorsLong {MapZone {Position {516.03f, 808.03f, 65.79f}, Position {1232.43f, 1195.97f, 64.02f}},
                                OUTSIDE_DOORS_LONG};
    _zones.push_back(outsideDoorsLong);

    NamedZone nearDoorsLong {MapZone {Position {968.03f, 215.03f, 75.7f}, Position {1227.96f, 769.19f, 71.39f}},
                             NEAR_DOORS_LONG};
    _zones.push_back(nearDoorsLong);

    NamedZone pit {MapZone {Position {1292.03f, 741.21f, 48.86f}, Position {1571.97f, 201.03f, -117.03f}}, PIT};
    _zones.push_back(pit);

    NamedZone farLong {MapZone {Position {1271.77f, 804.34f, 60.56f}, Position {1576.3f, 1594.73f, 64.05f}}, FAR_LONG};
    _zones.push_back(farLong);

    NamedZone aSiteLong {MapZone {Position {1289.37f, 1691.23f, 64.13f}, Position {1601.38f, 2306.24f, 68.40f}},
                         A_SITE_LONG};
    _zones.push_back(aSiteLong);

    NamedZone ramp {MapZone {Position {1601.38f, 2306.24f, 68.40f}, Position {1311.11f, 2772.2f, 179.13f}}, RAMP};
    _zones.push_back(ramp);

    NamedZone aSite {MapZone {Position {1235.97f, 2348.03f, 163.02f}, Position {1051.03f, 3059.97f, 194.33f}}, A_SITE};
    _zones.push_back(aSite);
    aSite.zone.obstacles.emplace_back(Position {1264.87f, 2460.97f, 191.03f}, Position {1176.88f, 2561.03f, 159.96f});
    aSite.zone.obstacles.emplace_back(Position {1097.64f, 2575.82f, 160.12f}, Position {989.21f, 2411.97f, 191.09f});
  }

  auto findZone(Position const& position) const -> NamedZone {
    auto zoneCompare = [position](NamedZone const& z1, NamedZone const& z2) {
      return z1.zone.volume.distance(position) < z2.zone.volume.distance(position);
    };
    return *std::ranges::min_element(_zones.begin(), _zones.end(), zoneCompare);
  }

private:
  std::map<MapZone, Transition> _transitions;
  std::vector<NamedZone> _zones;
};

} // namespace gabe
