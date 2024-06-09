//
// Created by stefan on 6/5/24.
//

#pragma once

#include "JsonUpdatable.hpp"
#include "engine/Weapon.hpp"
#include "utils/logger/Logger.hpp"
#include <CDS/util/JSON>
#include <array>

namespace gabe {
class Inventory : public JsonUpdatable<Inventory> {
public:
  enum class ActiveWeaponState { READY, RELOADING };
  enum class WeaponClass : unsigned int { WC_PRIMARY = 0, WC_PISTOL = 1, WC_KNIFE = 2, WC_BOMB = 3 };
  struct InventoryWeapon {
    Weapon weapon {};
    int ammo {};
    char correspondingKey {};

    [[nodiscard]] auto toString() const -> std::string { return weapon.toString(); }
  };

  [[nodiscard]] auto currentWeapon() const -> InventoryWeapon const& {
    return _weapons[static_cast<int>(_activeWeaponClass)];
  }

  [[nodiscard]] auto currentWeaponState() const -> ActiveWeaponState { return _activeWeaponState; }
  [[nodiscard]] auto currentWeaponClass() const -> WeaponClass { return _activeWeaponClass; }

  [[nodiscard]] auto weapons() const { return _weapons; }

  auto jsonUpdate(cds::json::JsonObject const& jsonObject) noexcept(false) -> void {
    using namespace cds;

    using enum WeaponClass;
    using E = MapEntry<StringView, WeaponClass>;
    static auto const typeMatcher =
        cds::mapOf(E {"Knife", WC_KNIFE}, E {"Pistol", WC_PISTOL}, E {"C4", WC_BOMB}, E {"Rifle", WC_PRIMARY});

    using enum ActiveWeaponState;
    using F = MapEntry<StringView, ActiveWeaponState>;
    static auto const stateMatcher = cds::mapOf(F {"reloading", RELOADING}, F {"active", READY});

    using G = MapEntry<StringView, Weapon>;
    static auto const weaponMatcher = cds::mapOf(G {"weapon_glock", GLOCK}, G {"weapon_ak47", AK_47});

    try {
      for (auto const& newWeapon : jsonObject.getJson("player").getJson("weapons")) {
        auto weaponType = typeMatcher[newWeapon.value().getJson().getString("type")];
        switch (weaponType) {
          case WC_PRIMARY:
          case WC_PISTOL: {
            auto weapon = weaponMatcher[newWeapon.value().getJson().getString("name")];
            _weapons[static_cast<unsigned int>(weaponType)].ammo = newWeapon.value().getJson().getInt("ammo_clip");
            _weapons[static_cast<unsigned int>(weaponType)].weapon = weapon;
            break;
          }
          default: {
            break;
          }
        }
        if (newWeapon.value().getJson().getString("state") != "holstered") {
          _activeWeaponClass = weaponType;
          _activeWeaponState = stateMatcher[newWeapon.value().getJson().getString("state")];
        }
      }
    } catch (cds::KeyException const& e) {
      log("Exception encountered while updating player inventory " + e.toString(), OpState::FAILURE);
      throw e;
    }
  }

private:
  ActiveWeaponState _activeWeaponState {};
  WeaponClass _activeWeaponClass {WeaponClass::WC_PISTOL};
  std::array<InventoryWeapon, 4> _weapons {{{NO_WEAPON, 0, '1'}, {NO_WEAPON, 0, '2'}, {KNIFE, 1, '3'}, {BOMB, 1, '4'}}};
};
} // namespace gabe
