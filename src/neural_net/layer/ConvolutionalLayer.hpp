//
// Created by stefan on 3/17/24.
//

#pragma once

#include "utils/math/function/Function.hpp"
#include "utils/math/linearArray/LinearArray.hpp"
namespace gabe::nn {
namespace impl {
template <typename DataType, typename InputType, typename DepthDim, typename KernelDim, typename StrideDim>
class ConvolutionalLayer :
    private utils::math::ConvolutionFunction<typename InputType::InnerLinearArray,
                                             utils::math::Kernel<DataType, KernelDim::size>, StrideDim::size> {
public:
  static constexpr Size kernelSize = KernelDim::size;
  static constexpr Size stride = StrideDim::size;
  static constexpr Size depth = DepthDim::size;
  static constexpr Size input_depth = InputType::first_size;

private:
  using KernelType = utils::math::Kernel<DataType, kernelSize>;
  using KernelArrayType = utils::math::LinearArray<DataType, depth, kernelSize, kernelSize>;
  using ActivationFunction = utils::math::ConvolutionFunction<typename InputType::InnerLinearArray, KernelType, stride>;

public:
  static constexpr auto outputSize =
      std::invoke_result_t<ActivationFunction, typename InputType::InnerLinearArray, KernelType> {}.size();
  static constexpr Size dimension = input_depth * depth * outputSize * outputSize;

  template <typename = void> using OutputType =
      utils::math::LinearArray<DataType, input_depth * depth, outputSize, outputSize>;

  ConvolutionalLayer() = default;
  ConvolutionalLayer(ConvolutionalLayer const&) = default;
  ConvolutionalLayer(ConvolutionalLayer&&) noexcept = default;

  auto feedForward(InputType const& input, KernelArrayType const& kernels) -> OutputType<> {
    OutputType<> rez {};
    auto idx = 0;
    for (auto const& mtrx : input) {
      for (auto const& kernel : kernels) {
        rez[idx++] = static_cast<ActivationFunction*>(this)->operator()(mtrx, kernel);
      }
    }
    return rez;
  }

  auto backPropagate(ConvolutionalLayer const& input) {
    return input.project([this]<typename T>(T&& value) {
      return static_cast<ActivationFunction*>(this)->derive(std::forward<T>(value));
    });
  }
};
} // namespace impl

template <Size depth, Size kernelSize, Size stride> struct ConvolutionalLayer {
  template <typename DataType, typename InputType> using Type =
      impl::ConvolutionalLayer<DataType, InputType, gabe::nn::impl::Dimension<depth>, gabe::nn::impl::Dimension<depth>,
                               gabe::nn::impl::Dimension<depth>>;
};

template <Size inputSize, Size depthSize> class ConvolutionalInputLayer {
public:
  template <typename, typename> using Type = ConvolutionalInputLayer;

  template <typename DataType> using OutputType = utils::math::LinearArray<DataType, depthSize, inputSize, inputSize>;
};

} // namespace gabe::nn
