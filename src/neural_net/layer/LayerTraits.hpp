//
// Created by stefan on 2/20/24.
//

#pragma once

#include <type_traits>
#include <types.hpp>

namespace gabe::nn {

namespace impl {
template <Size, Size, typename> class BaseConvolutionalLayer;
template <typename, typename, typename, typename, typename> class ConvolutionalLayer;
} // namespace impl


template <Size, Size> class ConvolutionalInputLayer;

namespace impl {
template <Size s> struct Dimension {
  static constexpr auto size = s;
};
template <template <typename...> typename LayerType, typename... Params> struct NDL {
  template <typename DataType> using Type = LayerType<DataType, Params...>;
};

template <typename> struct IsNDL : std::false_type {};
template <template <typename...> typename LayerType, typename... Params> struct IsNDL<NDL<LayerType, Params...>> :
    std::true_type {};

template <typename T, bool = IsNDL<T>::value> struct NDLType {};
template <typename T> struct NDLType<T, true> {
  template <typename DT> using Type = typename T::template Type<DT>;
};
template <typename T> struct NDLType<T, false> {
  template <typename> using Type = T;
};

template <typename, typename = void> struct IsConvolutionalLayer : std::false_type {};
template <typename T> struct IsConvolutionalLayer<T, std::void_t<decltype(T::isConvolutionalLayer)>> :
    std::true_type {};
template <Size inputSize, Size depthSize> struct IsConvolutionalLayer<ConvolutionalInputLayer<inputSize, depthSize>> :
    std::true_type {};
template <typename DataType, typename InputType, typename DepthDim, typename KernelDim, typename ConvFunc>
struct IsConvolutionalLayer<ConvolutionalLayer<DataType, InputType, DepthDim, KernelDim, ConvFunc>> : std::true_type {};

} // namespace impl

template <Size s, template <typename...> typename L, typename... P> using SizedLayer =
    impl::NDL<L, P..., impl::Dimension<s>>;

} // namespace gabe::nn
