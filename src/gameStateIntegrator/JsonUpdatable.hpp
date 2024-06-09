//
// Created by stefan on 6/6/24.
//

#pragma once

#include <CDS/util/JSON>

namespace gabe {

template <typename D> class JsonUpdatable {
public:
  auto update(cds::json::JsonObject const& jsonObject) noexcept(false) -> void {
    static_cast<D*>(this)->jsonUpdate(jsonObject);
  }
};
} // namespace gabe
