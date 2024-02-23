//
// Created by stefan on 2/22/24.
//

#pragma once

#include "utils/concepts/Concepts.hpp"
#include <array>
#include <concepts>
#include <types.hpp>

namespace gabe::utils::math {
template <typename L, typename R = L> struct Equals {
  auto operator()(L const& lhs, R const& rhs) const -> bool { return &lhs == &rhs; }
};

template <typename L, typename R>
  requires concepts::EqualComparable<L, R> && (!(std::floating_point<L> && std::floating_point<R>) )
    && (!(concepts::RandomAccessFloatContainer<L> && concepts::RandomAccessFloatContainer<R>) )
struct Equals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool { return lhs == rhs; }
};

template <typename L, typename R>
  requires std::floating_point<L> && std::floating_point<R>
struct Equals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool {
    return ((lhs - rhs > 0) ? (lhs - rhs) : (rhs - lhs)) < 0.00001;
  }
};

template <typename L, typename R>
  requires concepts::RandomAccessFloatContainer<L> && concepts::RandomAccessFloatContainer<R>
struct Equals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    for (auto idx = 0; idx < lhs.size(); ++idx) {
      auto val = (lhs[idx] - rhs[idx] > 0) ? (lhs[idx] - rhs[idx]) : (rhs[idx] - lhs[idx]);
      if (val > 0.00001) {
        return false;
      }
    }
    return true;
  }
};

template <> struct Equals<void, void> {
  template <typename L, typename R> auto operator()(L&& lhs, R&& rhs) const -> bool {
    return Equals<std::remove_cvref_t<L>, std::remove_cvref_t<R>>()(std::forward(lhs), std::forward(rhs));
  }
};
} // namespace gabe::utils::math
