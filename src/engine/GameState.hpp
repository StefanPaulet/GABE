//
// Created by stefan on 5/8/24.
//

#pragma once

#include "Map.hpp"
#include <cassert>
#include <mutex>
#include <types.hpp>
#include <vector>

namespace gabe {

struct Shared {
  std::mutex lock;
};

struct Image {
  unsigned char* data;
};

struct SharedPosition : Position, Shared {
  SharedPosition() = default;
  explicit SharedPosition(Position const& other) : Position(other) {}

  auto& operator=(Position const& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
  }

  auto position() const -> Position { return Position {x, y, z}; }
};
struct SharedImage : Image, Shared {
  SharedImage() = default;
  explicit SharedImage(Image const& other) : Image(other) {}

  auto& operator=(Image&& other) {
    data = std::exchange(other.data, nullptr);
    return *this;
  }

  auto image() -> Image { return Image {data}; }
};
struct SharedOrientation : Orientation, Shared {
  SharedOrientation() : Orientation() {}
  explicit SharedOrientation(Orientation const& other) : Orientation(other) {}

  auto& operator=(Orientation const& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
  }

  auto orientation() const -> Orientation { return Orientation {x, y, z}; }
};


class GameState {
public:
  enum class Properties { POSITION, ORIENTATION, IMAGE };

  template <typename V> auto set(Properties property, V&& value) -> void {
    auto setter = [&value]<typename T>(T& target) {
      std::lock_guard lg {target.lock};
      target = std::forward<V>(value);
    };

    switch (property) {
      using enum Properties;
      case POSITION: {
        if constexpr (std::is_assignable_v<std::remove_cvref_t<decltype(_position)>, V>) {
          setter(_position);
        }
        break;
      }
      case ORIENTATION: {
        if constexpr (std::is_assignable_v<std::remove_cvref_t<decltype(_orientation)>, V>) {
          setter(_orientation);
        }
        break;
      }
      case IMAGE: {
        if constexpr (std::is_assignable_v<std::remove_cvref_t<decltype(_image)>, V>) {
          setter(_image);
        }
        break;
      }
      default: {
        assert(false && "Attempting to set unknown property");
      }
    }
  }

  auto position() -> Position {
    std::lock_guard lockGuard {_position.lock};
    return _position.position();
  }

  auto orientation() -> Orientation {
    std::lock_guard lockGuard {_orientation.lock};
    return _orientation.orientation();
  }

  auto image() -> Image {
    std::lock_guard lockGuard {_image.lock};
    return _image.image();
  }

private:
  SharedPosition _position {};
  SharedOrientation _orientation {};
  SharedImage _image {};

public:
  Map const map {};
  Map::NamedZone targetZone {Zone {}, Map::ZoneName::NO_ZONE};
  Map::NamedZone nextZone {Zone {}, Map::ZoneName::NO_ZONE};
};
} // namespace gabe
