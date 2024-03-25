//
// Created by stefan on 2/21/24.
//

#pragma once

#include <type_traits>

namespace gabe::utils::math::impl {
template <typename, typename = void> struct is_callable : std::false_type {};
template <typename T> struct is_callable<T, std::void_t<decltype(std::declval<T>()())>> : std::true_type {};

template <typename, typename = void> struct is_derivable : std::false_type {};
template <typename T> struct is_derivable<T, std::void_t<decltype(std::declval<T>().derive())>> : std::true_type {};

template <typename, typename = void> struct is_cost_function : std::false_type {};
template <typename T> struct is_cost_function<T, std::void_t<decltype(T::isCostFunction)>> : std::true_type {};

template <typename, typename = void> struct is_container_function : std::false_type {};
template <typename T> struct is_container_function<T, std::void_t<decltype(T::isContainerFunction)>> :
    std::true_type {};

template <typename, typename = void> struct is_convolution_function : std::false_type {};
template <typename T> struct is_convolution_function<T, std::void_t<decltype(T::isConvolutionFunction)>> :
    std::true_type {};
} // namespace gabe::utils::math::impl
