//
// Created by stefan on 3/17/24.
//

#pragma once

#include "utils/math/function/Function.hpp"
#include "utils/math/linearArray/LinearArray.hpp"
namespace gabe::nn {
namespace impl {
template <typename DataType, typename InputType, typename DepthDim, typename KernelDim, typename ConvolutionFunction>
class ConvolutionalLayer : private ConvolutionFunction {
public:
  static constexpr Size kernelSize = KernelDim::size();
  static constexpr Size depth = DepthDim::size();
  static constexpr Size inputDepth = InputType::size();

private:
  using KernelType = utils::math::Kernel<DataType, kernelSize>;
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
      rez[idx++] = (static_cast<ConvolutionFunction&>(*this))(input, kernel);
    }
    return rez;
  }

  auto backPropagate(ConvolutionalLayer const& input) {
    return input.project([this]<typename T>(T&& value) {
      return static_cast<ConvolutionFunction*>(this)->derive(std::forward<T>(value));
    });
  }
};

template <Size depth, Size kernelSize, typename D> struct BaseConvolutionalLayer {
  static constexpr bool isConvolutionalLayer = true;

  template <typename DataType, typename InputType, typename T = D> using Type =
      impl::ConvolutionalLayer<DataType, InputType, gabe::nn::impl::Dimension<depth>,
                               gabe::nn::impl::Dimension<kernelSize>,
                               typename T::template ConvFunction<DataType, InputType>>;
};

} // namespace impl

template <Size depth, Size kernelSize> struct ConvolutionalLayer :
    impl::BaseConvolutionalLayer<depth, kernelSize, ConvolutionalLayer<depth, kernelSize>> {
  template <typename DataType, typename InputType> using ConvFunction = gabe::utils::math::SimpleConvolutionFunction<
      InputType, gabe::utils::math::LinearArray<DataType, InputType::size(), kernelSize, kernelSize>>;

  template <typename DataType, typename InputType> using Type = impl::BaseConvolutionalLayer<
      depth, kernelSize, ConvolutionalLayer<depth, kernelSize>>::template Type<DataType, InputType, ConvolutionalLayer>;
};

template <Size depth, Size kernelSize, Size stride> struct StridedConvolutionalLayer :
    impl::BaseConvolutionalLayer<depth, kernelSize, StridedConvolutionalLayer<depth, kernelSize, stride>> {
  template <typename DataType, typename InputType> using ConvFunction = gabe::utils::math::StridedConvolutionFunction<
      stride, InputType, gabe::utils::math::LinearArray<DataType, InputType::size(), kernelSize, kernelSize>>;

  template <typename DataType, typename InputType> using Type = impl::BaseConvolutionalLayer<
      depth, kernelSize, StridedConvolutionalLayer>::template Type<DataType, InputType, StridedConvolutionalLayer>;
};

template <Size inputSize, Size depthSize> class ConvolutionalInputLayer {
public:
  template <typename, typename> using Type = ConvolutionalInputLayer;

  template <typename DataType> using OutputType = utils::math::LinearArray<DataType, depthSize, inputSize, inputSize>;
};

} // namespace gabe::nn
