//
// Created by stefan on 2/14/24.
//

#pragma once

#include "utils/concepts/Concepts.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace gabe::utils::math {

template <typename = void> struct RoundFloat {};

template <> struct RoundFloat<void> {
  template <typename InputType> auto operator()(InputType&& input) const -> InputType {
    return RoundFloat<std::remove_cvref_t<InputType>>()(std::forward<InputType>(input));
  }
};

template <std::floating_point InputType> struct RoundFloat<InputType> {
  auto operator()(InputType input) const -> InputType {
    constexpr auto precision = 10000;
    return roundf(input * precision) / precision;
  }
};


template <typename = void> struct SigmoidFunction {};

template <std::floating_point InputType> struct SigmoidFunction<InputType> {
  static constexpr auto isActivationFunction = true;
  auto operator()(InputType const& input) const -> double {
    return static_cast<InputType>(1) / (static_cast<InputType>(1) + std::exp(-input));
  }
  auto derive(InputType input) const -> double {
    auto aux = operator()(input);
    return aux * (static_cast<InputType>(1) - aux);
  }
};

template <> struct SigmoidFunction<void> {
  static constexpr auto isActivationFunction = true;

  template <typename InputType> auto operator()(InputType&& input) const {
    return SigmoidFunction<std::remove_cvref_t<InputType>>()(std::forward<InputType>(input));
  }

  template <typename InputType> auto derive(InputType&& input) const {
    return SigmoidFunction<std::remove_cvref_t<InputType>>().derive(std::forward<InputType>(input));
  }
};


template <typename InputType = void> struct IdentityFunction {
  auto operator()(InputType const& input) const -> InputType { return input; }
  auto derive(InputType const&) const -> InputType { return static_cast<InputType>(1); }
};

template <> struct IdentityFunction<void> {
  template <typename InputType> auto operator()(InputType&& input) const {
    return IdentityFunction<std::remove_cvref_t<InputType>>()(std::forward<InputType>(input));
  }

  template <typename InputType> auto derive(InputType&& input) const {
    return IdentityFunction<std::remove_cvref_t<InputType>>().derive(std::forward<InputType>(input));
  }
};


template <typename InputType = void, typename TargetType = void> struct MeanSquaredErrorFunction {};

template <> struct MeanSquaredErrorFunction<void, void> {
  static constexpr auto isCostFunction = true;

  template <typename InputType, typename TargetType = InputType>
  auto operator()(InputType&& in, TargetType&& target) const {
    return MeanSquaredErrorFunction<std::remove_cvref_t<InputType>, std::remove_cvref_t<TargetType>>()(
        std::forward<InputType>(in), std::forward<TargetType>(target));
  }

  template <typename InputType, typename TargetType = InputType>
  auto derive(InputType&& in, TargetType&& target) const {
    return MeanSquaredErrorFunction<std::remove_cvref_t<InputType>, std::remove_cvref_t<TargetType>>().derive(
        std::forward<InputType>(in), std::forward<TargetType>(target));
  }
};

template <concepts::IntegralType InputType, concepts::IntegralType TargetType>
struct MeanSquaredErrorFunction<InputType, TargetType> {
  static constexpr auto isCostFunction = true;

  auto operator()(InputType in, TargetType target) const { return (target - in) * (target - in) / 2; }

  auto derive(InputType in, InputType target) const { return (in - target); }
};

template <concepts::LinearArrayType InputType, concepts::LinearArrayType TargetType>
struct MeanSquaredErrorFunction<InputType, TargetType> {
  static constexpr auto isCostFunction = true;

  auto operator()(InputType const& in, TargetType const& target) const {
    auto divider = [](typename InputType::UnderlyingType const& val) {
      return val / static_cast<typename InputType::UnderlyingType>(2);
    };
    return ((target - in) * (target - in)).project(divider);
  }

  auto derive(InputType const& in, TargetType const& target) const { return (in - target); }
};


template <typename = void> struct SoftmaxFunction {};

template <> struct SoftmaxFunction<void> {
  static constexpr auto isActivationFunction = true;
  static constexpr auto isContainerFunction = true;

  template <typename InputType> auto operator()(InputType&& input) const {
    return SoftmaxFunction<std::remove_cvref_t<InputType>>()(std::forward<InputType>(input));
  }

  template <typename InputType> auto derive(InputType&& input) const {
    return SoftmaxFunction<std::remove_cvref_t<InputType>>().derive(std::forward<InputType>(input));
  }
};

template <concepts::LinearArrayType InputType> struct SoftmaxFunction<InputType> {
  static constexpr auto isActivationFunction = true;
  static constexpr auto isContainerFunction = true;

  auto operator()(InputType const& input) const -> InputType {
    auto maxValue = input.max();
    auto expArray = input.project([maxValue](InputType::UnderlyingType v) { return expf(v - maxValue); });
    auto sum =
        expArray.accumulate(0, [](InputType::UnderlyingType lhs, InputType::UnderlyingType rhs) { return lhs + rhs; });
    return expArray.transform([sum](InputType::UnderlyingType v) { return RoundFloat<>()(v / sum); });
  }

  auto derive(InputType const& input) const -> InputType {
    auto aux = operator()(input);
    return aux * (InputType::unit() - aux);
  }
};


template <typename = void> struct StringToIntegral {};

template <std::integral T> struct StringToIntegral<T> {
  auto operator()(std::string const& str) const -> T { return strtol(str.c_str(), nullptr, 10); }
};

template <std::floating_point T> struct StringToIntegral<T> {
  auto operator()(std::string const& str) const -> T { return strtod(str.c_str(), nullptr); }
};

template <std::integral T, concepts::LinearArrayType OutputArrayType, typename LabelTransform = IdentityFunction<>>
struct OneHotEncoder {
  auto operator()(T value) -> OutputArrayType {
    OutputArrayType rez = OutputArrayType::nul();
    auto transformedValue = LabelTransform()(value);
    assert(transformedValue <= rez.size() && "Cannot one-hot encode a value which doesn't fit in the output array");
    rez[transformedValue] = rez[transformedValue].unit();
    return rez;
  }
};

template <std::integral T, concepts::LinearColumnArrayType InputArrayType, typename LabelTransform = IdentityFunction<>>
struct SoftMaxDecoder {
  auto operator()(InputArrayType const& input) -> T {
    auto max = input[0][0];
    Size maxIdx = 0;
    for (auto idx = 0; idx < input.size(); ++idx) {
      if (max < input[idx][0]) {
        maxIdx = idx;
        max = input[idx][0];
      }
    }
    return LabelTransform()(maxIdx);
  }
};

namespace impl {
template <concepts::DeepLinearMatrixType InputType, concepts::DeepLinearMatrixType KernelType, typename ConvType>
struct ConvolutionFunction {
  static constexpr auto isConvolutionFunction = true;

  static_assert(InputType::size() == KernelType::size(), "Cannot convolve a matrix with a kernel of different depth");

  auto operator()(InputType const& in, KernelType const& kernel) const {
    using ConvResultType = decltype(std::declval<ConvType>().convolve(
        std::declval<std::add_lvalue_reference_t<typename InputType::InnerLinearArray>>(),
        std::declval<std::add_lvalue_reference_t<typename KernelType::InnerLinearArray>>()));

    constexpr auto rezLineSize = ConvResultType::size();
    constexpr auto rezColumnSize = ConvResultType::total_size() / rezLineSize;

    LinearArray<typename InputType::UnderlyingType, rezLineSize, rezColumnSize> result {};
    for (auto idx = 0; idx < InputType::size(); ++idx) {
      result += static_cast<ConvType const*>(this)->convolve(in[idx], kernel[idx]);
    }
    return result;
  }
};

template <concepts::DeepLinearMatrixType InputType, typename PoolType> struct PoolingFunction {
  static constexpr auto isPoolingFunction = true;

  auto operator()(InputType const& in) const {
    using PoolResultType = decltype(std::declval<PoolType>().pool(
        std::declval<std::add_lvalue_reference_t<typename InputType::InnerLinearArray>>()));

    constexpr auto rezLineDepth = InputType::size();
    constexpr auto rezLineSize = PoolResultType::size();
    constexpr auto rezColumnSize = PoolResultType::total_size() / rezLineSize;

    LinearArray<typename InputType::UnderlyingType, rezLineDepth, rezLineSize, rezColumnSize> result {};
    for (auto idx = 0; idx < InputType::size(); ++idx) {
      result[idx] = static_cast<PoolType const*>(this)->pool(in[idx]);
    }
    return result;
  }
};
} // namespace impl

template <concepts::DeepLinearMatrixType InputType, concepts::DeepLinearMatrixType KernelType>
struct SimpleConvolutionFunction :
    impl::ConvolutionFunction<InputType, KernelType, SimpleConvolutionFunction<InputType, KernelType>> {
  auto convolve(typename InputType::InnerLinearArray const& in,
                typename KernelType::InnerLinearArray const& kernel) const {
    return in.convolve(kernel);
  }
};

template <Size stride, concepts::DeepLinearMatrixType InputType, concepts::DeepLinearMatrixType KernelType>
struct StridedConvolutionFunction :
    impl::ConvolutionFunction<InputType, KernelType, StridedConvolutionFunction<stride, InputType, KernelType>> {
  auto convolve(typename InputType::InnerLinearArray const& in,
                typename KernelType::InnerLinearArray const& kernel) const {
    return in.template stridedConvolve<stride>(kernel);
  }
};

namespace impl {
template <concepts::DeepLinearMatrixType InputType, typename PoolDim, typename StrideDim> struct MaxPoolFunction :
    impl::PoolingFunction<InputType, MaxPoolFunction<InputType, PoolDim, StrideDim>> {
  auto pool(typename InputType::InnerLinearArray const& in) const {
    auto predicate = [](typename InputType::UnderlyingType lhs, typename InputType::UnderlyingType rhs) {
      return lhs < rhs;
    };
    return in.template pool<PoolDim::size(), PoolDim::size(), StrideDim::size()>(predicate);
  }
};
} // namespace impl

template <concepts::DeepLinearMatrixType InputType, Size poolSize, Size stride> struct MaxPoolFunction :
    impl::MaxPoolFunction<InputType, gabe::nn::impl::Dimension<poolSize>, gabe::nn::impl::Dimension<stride>> {};

template <typename = void> struct ReluFunction {};
template <> struct ReluFunction<void> {
  static constexpr auto isActivationFunction = true;

  template <typename InputType> auto operator()(InputType&& in) {
    return ReluFunction<std::remove_cvref_t<InputType>> {}(std::forward<InputType>(in));
  }

  template <typename InputType> auto derive(InputType&& in) {
    return ReluFunction<std::remove_cvref_t<InputType>> {}.derive(std::forward<InputType>(in));
  }
};

template <concepts::IntegralType T> struct ReluFunction<T> {
  static constexpr auto isActivationFunction = true;

  auto operator()(T value) -> T { return std::max(value, static_cast<T>(0)); }

  auto derive(T value) -> T {
    if (value <= static_cast<T>(0)) {
      return 0;
    }
    return 1;
  }
};


namespace func {
template <typename InputType> constexpr SigmoidFunction<InputType> sigmoid;
} // namespace func
} // namespace gabe::utils::math
