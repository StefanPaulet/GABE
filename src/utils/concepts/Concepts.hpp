//
// Created by stefan on 2/23/24.
//

#pragma once

#include "types.hpp"
#include "utils/math/function/FunctionTraits.hpp"
#include "utils/math/linearArray/LinearArrayTraits.hpp"
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
concept Iterable = Iterator<std::remove_cvref_t<decltype(std::declval<T>().begin())>>;

template <typename T>
concept RandomAccessContainer =
    Iterable<T> && std::random_access_iterator<std::remove_cvref_t<decltype(std::declval<T>().begin())>>
    && requires(T t, Size idx) {
         { t[idx] } -> std::same_as<decltype(*t.begin())>;
       };

namespace impl {
template <typename T, typename = void> struct UseFloatingEquals : std::false_type {};

template <std::floating_point T> struct UseFloatingEquals<T> : std::true_type {};

template <gabe::utils::concepts::Iterable T> struct UseFloatingEquals<
    T, std::enable_if_t<UseFloatingEquals<std::remove_cvref_t<decltype(*std::declval<T>().begin())>>::value, void>> :
    std::true_type {};

template <typename> struct UsesDefaultEqOp : std::true_type {};
template <concepts::Iterable I> struct UsesDefaultEqOp<I> : std::false_type {};
template <std::floating_point F> struct UsesDefaultEqOp<F> : std::false_type {};
template <concepts::RandomAccessContainer C> struct UsesDefaultEqOp<C> : std::false_type {};
} // namespace impl

template <typename T>
concept UsesDefaultEqOp = impl::UsesDefaultEqOp<T>::value && EqualComparable<T>;

template <typename T>
concept IntegralType = std::integral<T> || std::floating_point<T>;

template <typename T>
concept LinearArrayType = gabe::utils::math::linearArray::impl::IsLinearArray<T>::value;

template <typename T>
concept LinearColumnArrayType =
    LinearArrayType<T> && gabe::utils::math::linearArray::impl::IsLinearColumnArray<T>::value;

template <typename T>
concept LinearMatrixType = LinearArrayType<T> && gabe::utils::math::linearArray::impl::IsLinearMatrix<T>::value;

template <typename T>
concept ContainerFunctionType = gabe::utils::math::impl::is_container_function<T>::value;
} // namespace gabe::utils::concepts
