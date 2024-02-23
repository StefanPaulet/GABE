//
// Created by stefan on 2/23/24.
//

#pragma once

#include "types.hpp"
#include <concepts>
#include <type_traits>

namespace gabe::utils::concepts {
template <typename L, typename R = L>
concept EqualComparable = requires(L lhs, R rhs) {
  { lhs == rhs } -> std::same_as<bool>;
};

template <typename T, typename D>
concept RandomAccessContainer = requires(T t, Size s) {
  { std::remove_cvref_t<decltype(t[s])>() } -> std::same_as<D>;
  { t.size() } -> std::convertible_to<int>;
};

template <typename T>
concept RandomAccessFloatContainer =
    RandomAccessContainer<T, typename T::value_type> && std::floating_point<typename T::value_type>;
} // namespace gabe::utils::concepts
