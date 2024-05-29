//
// Created by stefan on 5/25/24.
//

#pragma once

#include <algorithm>
#include <queue>

namespace gabe {

template <typename Policy> struct MovementPolicy {
  using SubZone = Zone::SubZone;

  [[nodiscard]] auto getNextPoint(Position const& startPoint, Zone const& startZone,
                                  std::vector<Zone> const& path) const -> Position {
    return static_cast<Policy const*>(this)->getPoint(startPoint, startZone, path);
  }

  [[nodiscard]] auto findTransitionZone(Zone const& start, Zone const& target) const -> std::vector<Zone> {
    auto transitions = map.possibleTransitions(start, target);
    std::vector<Zone> result {};
    for (auto const& t : transitions) {
      if (!t.transitionArea.empty()) {
        result.emplace_back(t.transitionArea);
      }
    }
    if (result.empty()) {
      result.emplace_back(start.volume.commonRegion(target.volume));
    }
    return result;
  }

  Map const& map;
};

struct DirectMovementPolicy : public MovementPolicy<DirectMovementPolicy> {
  [[nodiscard]] auto getPoint(Position const& startPoint, Zone const& startZone, std::vector<Zone> const& path) const
      -> Position {
    auto targetZone = path.back();
    auto targetSubzones = targetZone.subzones();
    auto transitionAreas = findTransitionZone(startZone, targetZone);
    auto currentSubzones = startZone.subzones();
    std::pair<int, int> indices {0, 0};

    auto currentZone = currentSubzones[0][0];
    for (auto lIdx = 0; lIdx < currentSubzones.size(); ++lIdx) {
      for (auto cIdx = 0; cIdx < currentSubzones[0].size(); ++cIdx) {
        if (currentSubzones[lIdx][cIdx].volume.distance(startPoint) < currentZone.volume.distance(startPoint)) {
          indices = {lIdx, cIdx};
          currentZone = currentSubzones[lIdx][cIdx];
        }
      }
    }
    for (auto const& ta : transitionAreas) {
      if (ta.volume.intersects(currentZone.volume)) {
        return ta.volume.closestPoint(startPoint);
      }
    }
    return astar(currentSubzones, transitionAreas, indices);
  }

  [[nodiscard]] auto astar(std::vector<std::vector<SubZone>> const& subzones, std::vector<Zone> const& transitionArea,
                           std::pair<int, int> const& startIndices) const -> Position {

    std::array<std::pair<int, int>, 4> neighbours {std::pair<int, int> {-1, 0}, std::pair<int, int> {0, 1},
                                                   std::pair<int, int> {1, 0}, std::pair<int, int> {0, -1}};
    std::map<SubZone, SubZone> parents {};
    std::map<SubZone, float> costs {};
    auto comparator = [&costs](SubZone const& z1, SubZone const& z2) { return costs.at(z1) > costs.at(z2); };
    std::priority_queue<SubZone, std::vector<SubZone>, decltype(comparator)> queue {comparator};

    auto startZone = subzones[startIndices.first][startIndices.second];
    costs[startZone] = 0;
    parents[startZone] = startZone;
    queue.push(startZone);

    while (!queue.empty()) {
      auto zone = queue.top();
      queue.pop();
      for (auto const& transition : transitionArea) {
        if (transition.volume.intersects(zone.volume)) {
          while (parents[zone] != startZone) {
            zone = parents[zone];
          }
          return zone.volume.center();
        }
      }
      for (auto const& neighbour : neighbours) {
        auto [line, column] =
            std::pair<int, int> {neighbour.first + zone.indices.first, neighbour.second + zone.indices.second};
        if (line >= 0 && line < subzones.size() && column >= 0 && column < subzones[0].size()) {
          auto newZone = subzones[line][column];
          if (!parents.contains(newZone) || costs[newZone] > costs[zone] + heuristic(newZone) + 1) {
            costs[newZone] = costs[zone] + heuristic(newZone) + 1;
            queue.push(newZone);
            parents[newZone] = zone;
          }
        }
      }
    }
    return Position {};
  }

  [[nodiscard]] auto heuristic(SubZone const& subzone) const -> float {
    if (subzone.accessible) {
      return 0;
    }
    return 100;
  }
};

} // namespace gabe