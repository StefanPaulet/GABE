//
// Created by stefan on 5/18/24.
//

#pragma once

#include "Map.hpp"
#include <queue>

namespace gabe::path {

template <typename Policy> struct PathFindingPolicy {
  auto computePath(Map::ZoneName startZone, Map::ZoneName targetZone) -> void {
    static_cast<Policy*>(this)->getPath(startZone, targetZone);
  }
  Map const& map;
  std::vector<Map::ZoneName> path;
};

struct ShortestPathPolicy : public PathFindingPolicy<ShortestPathPolicy> {
  auto getPath(Map::ZoneName startZone, Map::ZoneName targetZone) -> void {
    path.clear();

    if (startZone == targetZone) {
      path.push_back(startZone);
      return;
    }

    auto startMapZone = map.getZone(startZone);
    auto targetMapZone = map.getZone(targetZone);

    std::map<MapZone, MapZone> parents {};
    std::queue<MapZone> zoneQueue {};
    zoneQueue.push(startMapZone);
    parents[startMapZone] = startMapZone;
    while (true) {
      auto currZone = zoneQueue.front();
      zoneQueue.pop();
      for (auto const& zone : map.transitions().at(currZone)) {
        if (!parents.contains(zone.zone)) {
          if (zone.zone == targetMapZone) {
            path.push_back(targetZone);
            while (currZone != startMapZone) {
              path.push_back(map.getZoneName(currZone));
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
} // namespace gabe::path
