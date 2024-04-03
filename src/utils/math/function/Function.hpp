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
private:
  template <typename T = ConvType> using ConvResultType = typename T::ResultingConvolutionType;

public:
  static constexpr auto isConvolutionFunction = true;

  auto operator()(InputType const& in, KernelType const& kernel) const {

    static constexpr auto rezLineSize = ConvResultType<>::size();
    static constexpr auto rezColumnSize = ConvResultType<>::total_size() / rezLineSize;

    static_assert(InputType::size() == KernelType::size(), "Cannot convolve a matrix with a kernel of different depth");

    LinearArray<typename InputType::UnderlyingType, rezLineSize, rezColumnSize> result {};
    for (auto idx = 0; idx < InputType::size(); ++idx) {
      result += static_cast<ConvType const*>(this)->convolve(in[idx], kernel[idx]);
    }
    return result;
  }

  template <typename T = ConvType>
  auto derive(InputType const& in, typename T::ResultingConvolutionType const& nextLayerGradient) const {
    return static_cast<ConvType const*>(this)->deriveConvolve(in, nextLayerGradient);
  }

  auto fullyConvolve(InputType const& in, KernelType const& kernel) const {
    return static_cast<ConvType const*>(this)->paddedConvolve(in, kernel);
  }
};

template <concepts::DeepLinearMatrixType InputType, typename PoolType> struct PoolingFunction {
private:
  template <typename T = PoolType> using ResultingPoolType = typename T::ResultingPoolType;

public:
  static constexpr auto isPoolingFunction = true;

  auto operator()(InputType const& in) const {
    constexpr auto rezLineDepth = InputType::size();
    constexpr auto rezLineSize = ResultingPoolType<>::size();
    constexpr auto rezColSize = ResultingPoolType<>::InnerLinearArray::size();

    LinearArray<typename InputType::UnderlyingType, rezLineDepth, rezLineSize, rezColSize> result {};
    for (auto idx = 0; idx < InputType::size(); ++idx) {
      result[idx] = static_cast<PoolType const*>(this)->pool(in[idx]);
    }
    return result;
  }

  template <typename T = PoolType, Size rezLineSize = T::ResultingPoolType::size(),
            Size rezColSize = T::ResultingPoolType::InnerLinearArray::size()>
  auto derive(InputType const& in,
              LinearArray<typename InputType::UnderlyingType, InputType::size(), rezLineSize, rezColSize> const&
                  gradient) const {
    InputType result {};
    for (auto idx = 0; idx < in.size(); ++idx) {
      result[idx] = static_cast<PoolType const*>(this)->derivedPool(in[idx], gradient[idx]);
    }
    return result;
  }
};
} // namespace impl

template <concepts::DeepLinearMatrixType InputType, concepts::DeepLinearMatrixType KernelType>
struct SimpleConvolutionFunction :
    impl::ConvolutionFunction<InputType, KernelType, SimpleConvolutionFunction<InputType, KernelType>> {
  using ResultingConvolutionType =
      decltype(std::declval<typename InputType::InnerLinearArray>().convolve(typename KernelType::InnerLinearArray {}));

  auto convolve(typename InputType::InnerLinearArray const& in,
                typename KernelType::InnerLinearArray const& kernel) const {
    return in.convolve(kernel);
  }

  auto deriveConvolve(InputType const& in,
                      decltype(std::declval<typename InputType::InnerLinearArray>().convolve(
                          std::declval<std::add_lvalue_reference_t<typename KernelType::InnerLinearArray>>()))
                          const& gradient) const {
    static_assert(InputType::size() == KernelType::size(),
                  "Cannot compute simple convolution derivative on input and kernel of different depths");
    KernelType result {};
    for (auto idx = 0; idx < in.size(); ++idx) {
      result[idx] = in[idx].convolve(gradient);
    }
    return result;
  }

  auto paddedConvolve(InputType const& in, KernelType const& kernel) const {
    constexpr auto linePad = KernelType::InnerLinearArray::size() / 2;
    constexpr auto colPad = KernelType::InnerLinearArray::InnerLinearArray::size() / 2;

    using ResultingFeatureType =
        decltype(std::declval<typename InputType::InnerLinearArray>().template pad<linePad, colPad>());
    LinearArray<typename InputType::UnderlyingType, InputType::size(), ResultingFeatureType::size(),
                ResultingFeatureType::InnerLinearArray::size()>
        rez {};
    for (auto idx = 0; idx < in.size(); ++idx) {
      rez[idx] = in[idx].template pad<linePad, colPad>();
    }
    return SimpleConvolutionFunction<decltype(rez), KernelType> {}(rez, kernel);
  }
};

template <Size stride, concepts::DeepLinearMatrixType InputType, concepts::DeepLinearMatrixType KernelType>
struct StridedConvolutionFunction :
    impl::ConvolutionFunction<InputType, KernelType, StridedConvolutionFunction<stride, InputType, KernelType>> {
  using ResultingConvolutionType =
      decltype(std::declval<typename InputType::InnerLinearArray>().template stridedConvolve<stride>(
          typename KernelType::InnerLinearArray {}));

  auto convolve(typename InputType::InnerLinearArray const& in,
                typename KernelType::InnerLinearArray const& kernel) const {
    return in.template stridedConvolve<stride>(kernel);
  }
};

namespace impl {
template <concepts::DeepLinearMatrixType InputType, typename PoolDim, typename StrideDim> struct MaxPoolFunction :
    impl::PoolingFunction<InputType, MaxPoolFunction<InputType, PoolDim, StrideDim>> {

  static constexpr auto predicate = [](typename InputType::UnderlyingType lhs, typename InputType::UnderlyingType rhs) {
    return lhs < rhs;
  };

  using ResultingPoolType =
      decltype(std::declval<typename InputType::InnerLinearArray>()
                   .template pool<PoolDim::size(), PoolDim::size(), StrideDim::size()>(decltype(predicate) {}));

  auto pool(typename InputType::InnerLinearArray const& in) const {
    return in.template pool<PoolDim::size(), PoolDim::size(), StrideDim::size()>(predicate);
  }

  auto derivedPool(typename InputType::InnerLinearArray const& in, ResultingPoolType const& gradient) const {
    constexpr auto inLines = InputType::InnerLinearArray::size();
    constexpr auto inCols = InputType::InnerLinearArray::InnerLinearArray::size();

    typename InputType::InnerLinearArray result {};
    auto localSearch = [&result, in, gradient](Size lineStartIndex, Size colStartIndex, Size gLineIndex,
                                               Size gColIndex) {
      for (auto lIdx = 0; lIdx < PoolDim::size(); ++lIdx) {
        for (auto cIdx = 0; cIdx < PoolDim::size(); ++cIdx) {
          if (in[lineStartIndex + lIdx][colStartIndex + cIdx] == gradient[gLineIndex][gColIndex]) {
            result[lineStartIndex + lIdx][colStartIndex + cIdx] = 1;
            return;
          }
        }
      }
    };

    for (auto lIdx = 0, gLIdx = 0; lIdx < inLines; lIdx += StrideDim::size(), ++gLIdx) {
      for (auto cIdx = 0, gColIdx = 0; cIdx < inCols; cIdx += StrideDim::size(), ++gColIdx) {
        localSearch(lIdx, cIdx, gLIdx, gColIdx);
      }
    }
    return result;
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
