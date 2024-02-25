//
// Created by stefan on 2/22/24.
//

#pragma once

#include "utils/concepts/Concepts.hpp"
#include <array>
#include <concepts>
#include <types.hpp>

namespace gabe::utils::math {
template <typename L = void, typename R = L, typename = void> struct Equals {
  auto operator()(L const& lhs, R const& rhs) const -> bool { return &lhs == &rhs; }
};

template <> struct Equals<void, void, void> {
  template <typename L, typename R> auto operator()(L&& lhs, R&& rhs) const -> bool {
    return Equals<std::remove_cvref_t<L>, std::remove_cvref_t<R>>()(std::forward<L>(lhs), std::forward<R>(rhs));
  }
};

template <typename L, typename R>
  requires concepts::EqualComparable<L, R> && (!(concepts::Iterable<L> && concepts::Iterable<R>) )
struct Equals<
    L, R,
    std::enable_if_t<!(concepts::impl::UseFloatingEquals<L>::value && concepts::impl::UseFloatingEquals<R>::value),
                     void>> {
  auto operator()(L const& lhs, R const& rhs) const -> bool { return lhs == rhs; }
};

template <typename L, typename R>
  requires std::floating_point<L> && std::floating_point<R>
struct Equals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool {
    return ((lhs - rhs > 0) ? (lhs - rhs) : (rhs - lhs)) < 0.00001;
  }
};

template <concepts::Iterable L, concepts::Iterable R> struct Equals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    auto lhsIt = lhs.begin();
    auto rhsIt = rhs.begin();
    for (; lhsIt != lhs.end() && rhsIt != rhs.end(); ++lhsIt, ++rhsIt) {
      auto partialRes = Equals<>()(*lhsIt, *rhsIt);
      if (!partialRes) {
        return false;
      }
    }
    if (lhsIt != lhs.end() || rhsIt != rhs.end()) {
      return false;
    }
    return true;
  }
};
} // namespace gabe::utils::math
