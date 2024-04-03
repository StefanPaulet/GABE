//
// Created by stefan on 3/17/24.
//

#pragma once

#include "utils/math/function/Function.hpp"
#include "utils/math/linearArray/LinearArray.hpp"
namespace gabe::nn {
namespace impl {
template <typename DataType, typename InputType, typename DepthDim, typename KernelDim, typename ConvolutionFunction,
          typename ActivationFunction>
class ConvolutionalLayer : private ConvolutionFunction, private ActivationFunction {
public:
  static constexpr Size kernelSize = KernelDim::size();
  static constexpr Size depth = DepthDim::size();
  static constexpr Size inputDepth = InputType::size();

private:
  using KernelArrayType = utils::math::LinearArray<DataType, depth, inputDepth, kernelSize, kernelSize>;

public:
  static constexpr auto outputSize =
      std::invoke_result_t<ConvolutionFunction, InputType, typename KernelArrayType::InnerLinearArray>::size();

  template <typename = void> using OutputType = utils::math::LinearArray<DataType, depth, outputSize, outputSize>;
  static constexpr Size dimension = OutputType<>::total_size();

  ConvolutionalLayer() = default;
  ConvolutionalLayer(ConvolutionalLayer const&) = default;
  ConvolutionalLayer(ConvolutionalLayer&&) noexcept = default;

  auto feedForward(InputType const& input, KernelArrayType const& kernels) -> OutputType<> {
    OutputType<> rez {};
    auto idx = 0;
    for (auto const& kernel : kernels) {
      rez[idx++] =
          (static_cast<ConvolutionFunction&>(*this))(input, kernel).transform(*static_cast<ActivationFunction*>(this));
    }
    return rez;
  }

  auto backPropagate(InputType const& input, KernelArrayType const& kernels, OutputType<>& nextLayerGradient) {
    KernelArrayType kernelGradient {};
    InputType inputGradient {};

    nextLayerGradient.transform([this]<typename T>(T&& value) {
      return static_cast<ActivationFunction*>(this)->derive(std::forward<T>(value));
    });

    for (auto idx = 0; idx < nextLayerGradient.size(); ++idx) {
      kernelGradient[idx] = (static_cast<ConvolutionFunction const*>(this))->derive(input, nextLayerGradient[idx]);
    }

    for (auto idx = 0; idx < kernels.size(); ++idx) {
      typename KernelArrayType::InnerLinearArray flippedKernel = kernels[idx];
      for (auto& feature : flippedKernel) {
        feature = feature.flip();
      }
      inputGradient[idx] += (static_cast<ConvolutionFunction*>(this))->fullyConvolve(nextLayerGradient, flippedKernel);
    }

    return std::make_pair(kernelGradient, inputGradient);
  }
};

template <Size depth, Size kernelSize, typename ActivationFunction, typename D> struct BaseConvolutionalLayer {
  static constexpr bool isConvolutionalLayer = true;

  template <typename DataType, typename InputType, typename T = D> using Type =
      impl::ConvolutionalLayer<DataType, InputType, gabe::nn::impl::Dimension<depth>,
                               gabe::nn::impl::Dimension<kernelSize>,
                               typename T::template ConvFunction<DataType, InputType>, ActivationFunction>;
};

template <typename DataType, typename InputType, typename Dim, typename StrideDim, typename PoolingFunction>
class PoolingLayer : private PoolingFunction {
public:
  static constexpr Size poolingSize = Dim::size();
  static constexpr Size inputDepth = InputType::size();

private:
  using PoolingKernelType = utils::math::LinearArray<DataType, poolingSize, poolingSize>;

public:
  static constexpr auto outputSize = std::invoke_result_t<PoolingFunction, InputType>::InnerLinearArray::size();

  template <typename = void> using OutputType = utils::math::LinearArray<DataType, inputDepth, outputSize, outputSize>;
  static constexpr Size dimension = OutputType<>::total_size();

  PoolingLayer() = default;
  PoolingLayer(PoolingLayer const&) = default;
  PoolingLayer(PoolingLayer&&) noexcept = default;

  auto feedForward(InputType const& input) -> OutputType<> { return (static_cast<PoolingFunction&>(*this))(input); }
  auto backPropagate(InputType const& input, OutputType<> const& nextLayerGradient) -> InputType {
    input.project(
        [this]<typename T>(T&& value) { return static_cast<PoolingFunction*>(this)->derive(std::forward<T>(value)); });
  }
};

template <Size size, Size stride, typename D> struct BasePoolingLayer {
  static constexpr bool isPoolingLayer = true;

  template <typename DataType, typename InputType, typename T = D> using Type =
      impl::PoolingLayer<DataType, InputType, gabe::nn::impl::Dimension<size>, gabe::nn::impl::Dimension<stride>,
                         typename T::template PoolingFunction<InputType>>;
};

} // namespace impl

template <Size depth, Size kernelSize, typename ActivationFunction> struct ConvolutionalLayer :
    impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction,
                                 ConvolutionalLayer<depth, kernelSize, ActivationFunction>> {
  template <typename DataType, typename InputType> using ConvFunction = gabe::utils::math::SimpleConvolutionFunction<
      InputType, gabe::utils::math::LinearArray<DataType, InputType::size(), kernelSize, kernelSize>>;

  template <typename DataType, typename InputType> using Type =
      impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction,
                                   ConvolutionalLayer<depth, kernelSize, ActivationFunction>>::
          template Type<DataType, InputType, ConvolutionalLayer>;
};

template <Size depth, Size kernelSize, Size stride, typename ActivationFunction> struct StridedConvolutionalLayer :
    impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction,
                                 StridedConvolutionalLayer<depth, kernelSize, stride, ActivationFunction>> {
  template <typename DataType, typename InputType> using ConvFunction = gabe::utils::math::StridedConvolutionFunction<
      stride, InputType, gabe::utils::math::LinearArray<DataType, InputType::size(), kernelSize, kernelSize>>;

  template <typename DataType, typename InputType> using Type =
      impl::BaseConvolutionalLayer<depth, kernelSize, ActivationFunction, StridedConvolutionalLayer>::template Type<
          DataType, InputType, StridedConvolutionalLayer>;
};

template <Size inputSize, Size depthSize> class ConvolutionalInputLayer {
public:
  template <typename, typename> using Type = ConvolutionalInputLayer;

  template <typename DataType> using OutputType = utils::math::LinearArray<DataType, depthSize, inputSize, inputSize>;
};

template <Size size, Size stride> struct MaxPoolLayer :
    impl::BasePoolingLayer<size, stride, MaxPoolLayer<size, stride>> {

  template <typename InputType> using PoolingFunction =
      gabe::utils::math::impl::MaxPoolFunction<InputType, impl::Dimension<size>, impl::Dimension<stride>>;
  template <typename DataType, typename InputType> using Type =
      impl::BasePoolingLayer<size, stride, MaxPoolLayer>::template Type<DataType, InputType>;
};

} // namespace gabe::nn
