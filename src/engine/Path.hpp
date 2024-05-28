//
// Created by stefan on 5/18/24.
//

#pragma once

#include "Map.hpp"
#include <queue>

namespace gabe {

template <typename Policy> struct PathFindingPolicy {
  auto computePath(Map::ZoneName startZone, Map::ZoneName targetZone) -> void {
    static_cast<Policy*>(this)->getPath(startZone, targetZone);
  }
  Map const& map;
  std::vector<Zone> path;
};

struct ShortestPathPolicy : public PathFindingPolicy<ShortestPathPolicy> {
  auto getPath(Map::ZoneName startZone, Map::ZoneName targetZone) -> void {
    path.clear();

    auto startMapZone = map.getZoneByName(startZone);
    auto targetMapZone = map.getZoneByName(targetZone);

    if (startZone == targetZone) {
      path.push_back(startMapZone.zone);
      return;
    }

    std::map<Zone, Zone> parents {};
    std::queue<Zone> zoneQueue {};
    zoneQueue.push(startMapZone.zone);
    parents[startMapZone.zone] = startMapZone.zone;
    while (!zoneQueue.empty()) {
      auto currZone = zoneQueue.front();
      zoneQueue.pop();
      for (auto const& zone : map.transitions().at(currZone)) {
        if (!parents.contains(zone.zone)) {
          if (zone.zone == targetMapZone.zone) {
            path.push_back(map.getZoneByName(targetZone).zone);
            while (currZone != startMapZone.zone) {
              path.push_back(map.getZoneByVolume(currZone).zone);
              currZone = parents[currZone];
            }
            return;
          }
          parents[zone.zone] = currZone;
          zoneQueue.push(zone.zone);
        }
      }
    }
  }
};
} // namespace gabe
