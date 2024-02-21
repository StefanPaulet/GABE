//
// Created by stefan on 2/21/24.
//

#pragma once

namespace gabe::utils::math::impl {
template <typename, typename = void> struct is_callable : std::false_type {};
template <typename T> struct is_callable<T, std::void_t<decltype(std::declval<T>()())>> : std::true_type {};

template <typename, typename = void> struct is_derivable : std::false_type {};
template <typename T> struct is_derivable<T, std::void_t<decltype(std::declval<T>().derive())>> : std::true_type {};
} // namespace gabe::utils::math::impl
