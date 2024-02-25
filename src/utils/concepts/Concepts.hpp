//
// Created by stefan on 2/23/24.
//

#pragma once

#include "types.hpp"
#include <concepts>
#include <iterator>
#include <type_traits>

namespace gabe::utils::concepts {
template <typename L, typename R = L>
concept EqualComparable = requires(L lhs, R rhs) {
  { lhs == rhs } -> std::same_as<bool>;
};

template <typename T>
concept IsNotVoid = !std::same_as<T, void>;

template <typename I>
concept Iterator = requires(I i) {
  { *i } -> IsNotVoid;
  { ++i } -> std::same_as<I&>;
} && std::copyable<I>;

template <typename T>
concept Iterable = requires(T t) {
  { t.begin() } -> Iterator;
  { t.size() } -> std::convertible_to<Size>;
};

template <typename T>
concept RandomAccessContainer = std::random_access_iterator<std::remove_cvref_t<decltype(std::declval<T>().begin())>>;

namespace impl {
template <typename T, typename = void> struct UseFloatingEquals : std::false_type {};

template <std::floating_point T> struct UseFloatingEquals<T> : std::true_type {};

template <gabe::utils::concepts::Iterable T> struct UseFloatingEquals<
    T, std::enable_if_t<UseFloatingEquals<std::remove_cvref_t<decltype(*std::declval<T>().begin())>>::value, void>> :
    std::true_type {};
} // namespace impl

} // namespace gabe::utils::concepts
