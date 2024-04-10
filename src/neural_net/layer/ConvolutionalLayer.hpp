//
// Created by stefan on 3/17/24.
//

#pragma once

#include "utils/math/function/Function.hpp"
#include "utils/math/linearArray/LinearArray.hpp"
namespace gabe::nn {
namespace impl {
template <typename DataType, typename Input, typename DepthDim, typename KernelDim, typename ConvolutionFunction,
          typename ActivationFunction, typename InitializationScheme>
class ConvolutionalLayer : private ActivationFunction, private ConvolutionFunction {
public:
  static constexpr Size kernelSize = KernelDim::size();
  static constexpr Size depth = DepthDim::size();
  static constexpr Size inputDepth = Input::size();

private:
  using KernelArrayType = utils::math::LinearArray<DataType, depth, inputDepth, kernelSize, kernelSize>;
  using KernelType = typename KernelArrayType::InnerLinearArray;

public:
  static constexpr auto outputSize =
      std::invoke_result_t<ConvolutionFunction, Input, typename KernelArrayType::InnerLinearArray>::size();

  template <typename = void> using OutputType = utils::math::LinearArray<DataType, depth, outputSize, outputSize>;
  static constexpr Size dimension = OutputType<>::total_size();

  using InitializationFunction = InitializationScheme;

  ConvolutionalLayer() = default;
  ConvolutionalLayer(ConvolutionalLayer const&) = default;
  ConvolutionalLayer(ConvolutionalLayer&&) noexcept = default;

  auto feedForward(Input const& input, KernelArrayType const& kernels) -> OutputType<> {
    OutputType<> rez {};
    auto idx = 0;
    for (auto const& kernel : kernels) {
      rez[idx++] =
          (static_cast<ConvolutionFunction&>(*this))(input, kernel).transform(*static_cast<ActivationFunction*>(this));
    }
    return rez;
  }

  auto backPropagate(Input const& input, KernelArrayType const& kernels, OutputType<>& nextLayerGradient) {
    KernelArrayType kernelGradient {};
    Input inputGradient {};

    nextLayerGradient.transform([this]<typename T>(T&& value) {
      return static_cast<ActivationFunction*>(this)->derive(std::forward<T>(value));
    });

    for (auto idx = 0; idx < nextLayerGradient.size(); ++idx) {
      kernelGradient[idx] = (static_cast<ConvolutionFunction*>(this))->derive(input, nextLayerGradient[idx]);
    }

    for (auto idx = 0; idx < kernels.size(); ++idx) {
      typename KernelArrayType::InnerLinearArray flippedKernel = kernels[idx];
      for (auto& feature : flippedKernel) {
        feature = feature.flip();
      }
      inputGradient += static_cast<ConvolutionFunction*>(this)->fullyConvolve(nextLayerGradient[idx], flippedKernel);
    }

    return std::make_pair(kernelGradient, inputGradient);
  }
};

template <Size depth, Size kernelSize, typename ActivationFunction, typename D> struct BaseConvolutionalLayer {
  static constexpr bool isConvolutionalLayer = true;

  template <typename DataType, typename Input, typename T = D> using Type =
      impl::ConvolutionalLayer<DataType, Input, gabe::nn::impl::Dimension<depth>, gabe::nn::impl::Dimension<kernelSize>,
                               typename T::template CF<DataType, Input>, ActivationFunction, typename T::IS>;
};

template <typename DataType, typename Input, typename Dim, typename StrideDim, typename PoolingFunction>
class PoolingLayer : private PoolingFunction {
public:
  static constexpr Size poolingSize = Dim::size();
  static constexpr Size inputDepth = Input::size();

private:
  using PoolingKernelType = utils::math::LinearArray<DataType, poolingSize, poolingSize>;

public:
  static constexpr auto outputSize = std::invoke_result_t<PoolingFunction, Input>::InnerLinearArray::size();

  template <typename = void> using OutputType = utils::math::LinearArray<DataType, inputDepth, outputSize, outputSize>;
  static constexpr Size dimension = OutputType<>::total_size();

  PoolingLayer() = default;
  PoolingLayer(PoolingLayer const&) = default;
  PoolingLayer(PoolingLayer&&) noexcept = default;

  auto feedForward(Input const& input) -> OutputType<> { return (static_cast<PoolingFunction&>(*this))(input); }
  auto backPropagate(Input const& input, OutputType<> const& nextLayerGradient) -> Input {
    return static_cast<PoolingFunction*>(this)->derive(input, nextLayerGradient);
  }
};

template <Size size, Size stride, typename D> struct BasePoolingLayer {
  static constexpr bool isPoolingLayer = true;

  template <typename DataType, typename Input, typename T = D> using Type =
      impl::PoolingLayer<DataType, Input, gabe::nn::impl::Dimension<size>, gabe::nn::impl::Dimension<stride>,
                         typename T::template PoolingFunction<Input>>;
};

} // namespace impl

template <Size depth, Size kernelSize, typename ActivationFunction, typename InitializationScheme = NoInitialization>
struct ConvolutionalLayer :
    impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction,
                                 ConvolutionalLayer<depth, kernelSize, ActivationFunction, InitializationScheme>> {
  template <typename DataType, typename Input> using CF = gabe::utils::math::SimpleDeepConvolutionFunction<
      Input, gabe::utils::math::LinearArray<DataType, Input::size(), kernelSize, kernelSize>>;

  using IS = InitializationScheme;

  template <typename DataType, typename Input> using Type =
      impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction,
                                   ConvolutionalLayer<depth, kernelSize, ActivationFunction, InitializationScheme>>::
          template Type<DataType, Input, ConvolutionalLayer>;
};

template <Size depth, Size kernelSize, typename ActivationFunction, typename InitializationScheme = NoInitialization>
struct FullConvolutionalLayer :
    impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction,
                                 FullConvolutionalLayer<depth, kernelSize, ActivationFunction, InitializationScheme>> {
  template <typename DataType, typename Input> using CF = gabe::utils::math::FullDeepConvolutionFunction<
      Input, gabe::utils::math::LinearArray<DataType, Input::size(), kernelSize, kernelSize>>;

  using IS = InitializationScheme;

  template <typename DataType, typename Input> using Type =
      impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction,
                                   FullConvolutionalLayer<depth, kernelSize, ActivationFunction,
                                                          InitializationScheme>>::template Type<DataType, Input,
                                                                                                FullConvolutionalLayer>;
};

template <Size depth, Size kernelSize, Size stride, typename ActivationFunction,
          typename InitializationScheme = NoInitialization>
struct StridedConvolutionalLayer :
    impl::BaseConvolutionalLayer<
        depth, kernelSize, ActivationFunction,
        StridedConvolutionalLayer<depth, kernelSize, stride, ActivationFunction, InitializationScheme>> {
  template <typename DataType, typename Input> using CF = gabe::utils::math::StridedDeepConvolutionFunction<
      stride, Input, gabe::utils::math::LinearArray<DataType, Input::size(), kernelSize, kernelSize>>;

  using IS = InitializationScheme;

  template <typename DataType, typename Input> using Type =
      impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction, StridedConvolutionalLayer>::template Type<
          DataType, Input, StridedConvolutionalLayer>;
};

template <Size depth, Size kernelSize, Size stride, typename ActivationFunction,
          typename InitializationScheme = NoInitialization>
struct FullStridedConvolutionalLayer :
    impl::BaseConvolutionalLayer<
        depth, kernelSize, ActivationFunction,
        FullStridedConvolutionalLayer<depth, kernelSize, stride, ActivationFunction, InitializationScheme>> {
  template <typename DataType, typename Input> using CF = gabe::utils::math::FullStridedDeepConvolutionFunction<
      stride, Input, gabe::utils::math::LinearArray<DataType, Input::size(), kernelSize, kernelSize>>;

  using IS = InitializationScheme;

  template <typename DataType, typename Input> using Type =
      impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction, FullStridedConvolutionalLayer>::template Type<
          DataType, Input, FullStridedConvolutionalLayer>;
};

template <Size inputSize, Size depthSize> class ConvolutionalInputLayer {
public:
  template <typename, typename> using Type = ConvolutionalInputLayer;

  template <typename DataType> using OutputType = utils::math::LinearArray<DataType, depthSize, inputSize, inputSize>;
};

template <Size size, Size stride> struct MaxPoolLayer :
    impl::BasePoolingLayer<size, stride, MaxPoolLayer<size, stride>> {

  template <typename Input> using PoolingFunction =
      gabe::utils::math::impl::MaxPoolFunction<Input, impl::Dimension<size>, impl::Dimension<stride>>;
  template <typename DataType, typename Input> using Type =
      impl::BasePoolingLayer<size, stride, MaxPoolLayer>::template Type<DataType, Input>;
};

} // namespace gabe::nn
