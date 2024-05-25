//
// Created by stefan on 5/25/24.
//

#pragma once

namespace gabe {

template <typename Policy> struct MovementPolicy {
  [[nodiscard]] auto getNextPoint(Position const& startPoint, Zone const& startZone,
                                  std::vector<Zone> const& path) const -> Position {
    return static_cast<Policy const*>(this)->getPoint(startPoint, startZone, path);
  }
};

struct DirectMovementPolicy : public MovementPolicy<DirectMovementPolicy> {
  [[nodiscard]] auto getPoint(Position const& startPoint, Zone const& startZone, std::vector<Zone> const& path) const
      -> Position {
    auto targetZone = path.back();
    auto targetPoint = targetZone.volume.center();
    auto connectingLine = Line {startPoint, targetPoint};
    auto commonRegion = startZone.volume.commonRegion(targetZone.volume);
    if (commonRegion.containsLine(connectingLine)) {
      return targetPoint;
    }
    auto firstPossibility = Position {startPoint.x, targetPoint.y, 0};
    if (startZone.volume.containsInXY(firstPossibility) || targetZone.volume.containsInXY(firstPossibility)) {
      return firstPossibility;
    }
    return Position {targetPoint.x, startPoint.y, startPoint.z};
  }
};

} // namespace gabe