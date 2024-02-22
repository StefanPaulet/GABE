//
// Created by stefan on 2/22/24.
//

#pragma once

#include <array>
#include <concepts>
#include <types.hpp>

namespace gabe::utils::math {

namespace impl {
template <typename L, typename R = L, typename = void> struct is_equal_comparable : std::false_type {};
template <typename L, typename R>
struct is_equal_comparable<L, R, std::void_t<decltype(std::declval<L>() == std::declval<R>())>> : std::true_type {};
} // namespace impl

template <typename L, typename R = L, std::enable_if_t<!impl::is_equal_comparable<L, R>::value, int> = 0>
auto equals(L const& lhs, R const& rhs) -> bool {
  return &lhs == &rhs;
}

template <typename L, typename R = L,
          std::enable_if_t<impl::is_equal_comparable<L, R>::value
                               && !(std::is_floating_point_v<L> && std::is_floating_point_v<R>),
                           int> = 0>
auto equals(L const& lhs, R const& rhs) -> bool {
  return lhs == rhs;
}

template <typename L, typename R = L,
          std::enable_if_t<std::is_floating_point_v<L> && std::is_floating_point_v<R>, int> = 0>
auto equals(L const& lhs, R const& rhs) -> bool {
  return ((lhs - rhs > 0) ? (lhs - rhs) : (rhs - lhs)) < 0.00001;
}

template <Size s, typename L, typename R = L,
          std::enable_if_t<std::is_floating_point_v<L> && std::is_floating_point_v<R>, int> = 0>
auto equals(std::array<L, s> const& lhs, std::array<R, s> const& rhs) -> bool {
  for (auto idx = 0; idx < s; ++idx) {
    auto val = (lhs[idx] - rhs[idx] > 0) ? (lhs[idx] - rhs[idx]) : (rhs[idx] - lhs[idx]);
    if (val > 0.00001) {
      return false;
    }
  }
  return true;
}
} // namespace gabe::utils::math
