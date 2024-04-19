//
// Created by stefan on 2/22/24.
//

#pragma once

#include "Predicates.hpp"
#include "utils/concepts/Concepts.hpp"
#include <array>
#include <concepts>
#include <types.hpp>

namespace gabe::utils::math {
template <typename L = void, typename R = L> struct Equals {
  auto operator()(L const& lhs, R const& rhs) const -> bool { return &lhs == &rhs; }
};

template <> struct Equals<void, void> {
  template <typename L, typename R> auto operator()(L&& lhs, R&& rhs) const -> bool {
    return Equals<std::remove_cvref_t<L>, std::remove_cvref_t<R>>()(std::forward<L>(lhs), std::forward<R>(rhs));
  }
};

template <concepts::UsesDefaultEqOp Lhs, concepts::UsesDefaultEqOp Rhs> struct Equals<Lhs, Rhs> {
  auto operator()(Lhs const& lhs, Rhs const& rhs) const -> bool { return lhs == rhs; }
};

template <std::floating_point L, std::floating_point R> struct Equals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool {
    return ((lhs - rhs > 0) ? (lhs - rhs) : (rhs - lhs)) < 0.0001;
  }
};

template <concepts::Iterable L, concepts::Iterable R> struct Equals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool {
    auto lhsIt = lhs.begin();
    auto rhsIt = rhs.begin();
    for (; lhsIt != lhs.end() && rhsIt != rhs.end(); ++lhsIt, ++rhsIt) {
      auto partialRes = Equals<>()(*lhsIt, *rhsIt);
      if (!partialRes) {
        return false;
      }
    }
    return true;
  }
};

template <concepts::RandomAccessContainer L, concepts::RandomAccessContainer R> struct Equals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    for (Size idx = 0; idx < lhs.size(); ++idx) {
      auto partialRes = Equals<>()(lhs[idx], rhs[idx]);
      if (!partialRes) {
        return false;
      }
    }
    return true;
  }
};


template <typename L = void, typename R = L> struct HighPrecisionEquals {
  auto operator()(L const& lhs, R const& rhs) const -> bool { return &lhs == &rhs; }
};

template <> struct HighPrecisionEquals<void, void> {
  template <typename L, typename R> auto operator()(L&& lhs, R&& rhs) const -> bool {
    return HighPrecisionEquals<std::remove_cvref_t<L>, std::remove_cvref_t<R>>()(std::forward<L>(lhs),
                                                                                 std::forward<R>(rhs));
  }
};

template <std::floating_point L, std::floating_point R> struct HighPrecisionEquals<L, R> {
  auto operator()(L const& lhs, R const& rhs) const -> bool {
    return ((lhs - rhs > 0) ? (lhs - rhs) : (rhs - lhs)) < 0.000000001;
  }
};
} // namespace gabe::utils::math
