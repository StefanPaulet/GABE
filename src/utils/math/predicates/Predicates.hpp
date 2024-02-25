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

template <typename> struct UsesDefaultEqOp : std::true_type {};
template <concepts::Iterable I> struct UsesDefaultEqOp<I> : std::false_type {};
template <std::floating_point F> struct UsesDefaultEqOp<F> : std::false_type {};
}

namespace gabe::utils::concepts {
template <typename T> concept UsesDefaultEqOp = math::UsesDefaultEqOp<T>::value && EqualComparable<T>;
}

namespace gabe::utils::math {
template <concepts::UsesDefaultEqOp Lhs, concepts::UsesDefaultEqOp Rhs> struct Equals<Lhs, Rhs> {
  auto operator()(Lhs const& lhs, Rhs const& rhs) const -> bool { return lhs == rhs; }
};

template <std::floating_point L, std::floating_point R> struct Equals<L, R> {
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
