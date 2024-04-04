//
// Created by stefan on 2/23/24.
//

#pragma once

#include "neural_net/layer/LayerTraits.hpp"
#include "types.hpp"
#include "utils/math/function/FunctionTraits.hpp"
#include "utils/math/linearArray/LinearArrayTraits.hpp"
#include <concepts>
#include <iterator>
#include <type_traits>

namespace gabe::nn::impl {
template <typename, typename, typename, typename...> class LayerPair;
}

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
concept DeepLinearMatrixType = LinearArrayType<T> && gabe::utils::math::linearArray::impl::IsDeepLinearMatrix<T>::value;

template <typename T>
concept ConvolutionalLayerType = nn::impl::IsConvolutionalLayer<T>::value;

template <typename T>
concept PoolingLayerType = nn::impl::IsPoolingLayer<T>::value;

template <typename T>
concept ThreeDimensionalLayerType = ConvolutionalLayerType<T> || PoolingLayerType<T>;

template <typename T>
concept ContainerFunctionType = gabe::utils::math::impl::is_container_function<T>::value;

template <typename T>
concept DeepConvolutionFunctionType = gabe::utils::math::impl::is_convolution_function<T>::value;

namespace impl {
template <typename, typename = void> struct IsConvolutionalLayerPair : std::false_type {};
template <typename DataType, gabe::utils::concepts::ConvolutionalLayerType FirstLayer,
          gabe::utils::concepts::ConvolutionalLayerType SecondLayer, typename... RemainingLayers>
struct IsConvolutionalLayerPair<gabe::nn::impl::LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...>> :
    std::true_type {};
} // namespace impl

template <typename T>
concept ConvolutionalLayerPairType = impl::IsConvolutionalLayerPair<T>::value;
} // namespace gabe::utils::concepts
