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
  template <typename Input> auto operator()(Input&& input) const -> Input {
    return RoundFloat<std::remove_cvref_t<Input>>()(std::forward<Input>(input));
  }
};

template <std::floating_point Input> struct RoundFloat<Input> {
  auto operator()(Input input) const -> Input {
    constexpr auto precision = 10000;
    return roundf(input * precision) / precision;
  }
};


template <typename = void> struct SigmoidFunction {};

template <std::floating_point Input> struct SigmoidFunction<Input> {
  static constexpr auto isActivationFunction = true;
  auto operator()(Input const& input) const -> double {
    return static_cast<Input>(1) / (static_cast<Input>(1) + std::exp(-input));
  }
  auto derive(Input input) const -> double {
    auto aux = operator()(input);
    return aux * (static_cast<Input>(1) - aux);
  }
};

template <> struct SigmoidFunction<void> {
  static constexpr auto isActivationFunction = true;

  template <typename Input> auto operator()(Input&& input) const {
    return SigmoidFunction<std::remove_cvref_t<Input>>()(std::forward<Input>(input));
  }

  template <typename Input> auto derive(Input&& input) const {
    return SigmoidFunction<std::remove_cvref_t<Input>>().derive(std::forward<Input>(input));
  }
};


template <typename Input = void> struct IdentityFunction {
  auto operator()(Input const& input) const -> Input { return input; }
  auto derive(Input const&) const -> Input { return static_cast<Input>(1); }
};

template <> struct IdentityFunction<void> {
  template <typename Input> auto operator()(Input&& input) const {
    return IdentityFunction<std::remove_cvref_t<Input>>()(std::forward<Input>(input));
  }

  template <typename Input> auto derive(Input&& input) const {
    return IdentityFunction<std::remove_cvref_t<Input>>().derive(std::forward<Input>(input));
  }
};


template <typename Input = void, typename Target = void> struct MeanSquaredErrorFunction {};

template <> struct MeanSquaredErrorFunction<void, void> {
  static constexpr auto isCostFunction = true;

  template <typename Input, typename Target = Input> auto operator()(Input&& in, Target&& target) const {
    return MeanSquaredErrorFunction<std::remove_cvref_t<Input>, std::remove_cvref_t<Target>>()(
        std::forward<Input>(in), std::forward<Target>(target));
  }

  template <typename Input, typename Target = Input> auto derive(Input&& in, Target&& target) const {
    return MeanSquaredErrorFunction<std::remove_cvref_t<Input>, std::remove_cvref_t<Target>>().derive(
        std::forward<Input>(in), std::forward<Target>(target));
  }
};

template <concepts::IntegralType Input, concepts::IntegralType Target> struct MeanSquaredErrorFunction<Input, Target> {
  static constexpr auto isCostFunction = true;

  auto operator()(Input in, Target target) const { return (target - in) * (target - in) / 2; }

  auto derive(Input in, Input target) const { return (in - target); }
};

template <concepts::LinearArrayType Input, concepts::LinearArrayType Target>
struct MeanSquaredErrorFunction<Input, Target> {
  static constexpr auto isCostFunction = true;

  auto operator()(Input const& in, Target const& target) const {
    auto divider = [](typename Input::UnderlyingType val) {
      return val / static_cast<typename Input::UnderlyingType>(2);
    };
    return ((target - in) * (target - in)).project(divider);
  }

  auto derive(Input const& in, Target const& target) const { return (in - target); }
};

template <typename Input = void, typename Target = void> struct CrossEntropyFunction {};

template <> struct CrossEntropyFunction<void, void> {
  static constexpr auto isCostFunction = true;

  template <typename Input, typename Target = Input> auto operator()(Input&& in, Target&& target) const {
    return CrossEntropyFunction<std::remove_cvref_t<Input>, std::remove_cvref_t<Target>>()(
        std::forward<Input>(in), std::forward<Target>(target));
  }

  template <typename Input, typename Target = Input> auto derive(Input&& in, Target&& target) const {
    return CrossEntropyFunction<std::remove_cvref_t<Input>, std::remove_cvref_t<Target>>().derive(
        std::forward<Input>(in), std::forward<Target>(target));
  }
};

template <concepts::LinearMatrixType Input, concepts::LinearMatrixType Target>
struct CrossEntropyFunction<Input, Target> {
  static constexpr auto isCostFunction = true;

  auto operator()(Input const& in, Target const& target) const {
    auto loger = [](typename Input::UnderlyingType val) { return std::log(val); };
    return target * in.project(loger) * -1;
  }

  auto derive(Input const& in, Target const& target) const { return target / in * -1; }
};

template <typename Input = void, typename Target = void> struct CategoricalCrossEntropyFunction {};

template <> struct CategoricalCrossEntropyFunction<void, void> {
  static constexpr auto isCostFunction = true;
  static constexpr auto isCatCrossEntropy = true;

  template <typename Input, typename Target = Input> auto operator()(Input&& in, Target&& target) const {
    return CategoricalCrossEntropyFunction<std::remove_cvref_t<Input>, std::remove_cvref_t<Target>>()(
        std::forward<Input>(in), std::forward<Target>(target));
  }

  template <typename Input, typename Target = Input> auto derive(Input&& in, Target&& target) const {
    return CategoricalCrossEntropyFunction<std::remove_cvref_t<Input>, std::remove_cvref_t<Target>>().derive(
        std::forward<Input>(in), std::forward<Target>(target));
  }
};

template <concepts::LinearMatrixType Input, concepts::LinearMatrixType Target>
struct CategoricalCrossEntropyFunction<Input, Target> {
  static constexpr auto isCostFunction = true;
  static constexpr auto isCatCrossEntropy = true;

  auto operator()(Input const& in, Target const& target) const {
    auto loger = [](typename Input::UnderlyingType val) { return std::log(val); };
    return target * in.project(loger) * -1;
  }

  auto derive(Input const& in, Target const& target) const { return in - target; }
};


template <typename = void> struct SoftmaxFunction {};

template <> struct SoftmaxFunction<void> {
  static constexpr auto isActivationFunction = true;
  static constexpr auto isContainerFunction = true;

  template <typename Input> auto operator()(Input&& input) const {
    return SoftmaxFunction<std::remove_cvref_t<Input>>()(std::forward<Input>(input));
  }

  template <typename Input> auto derive(Input&& input) const {
    return SoftmaxFunction<std::remove_cvref_t<Input>>().derive(std::forward<Input>(input));
  }
};

template <concepts::LinearArrayType Input> struct SoftmaxFunction<Input> {
  static constexpr auto isActivationFunction = true;
  static constexpr auto isContainerFunction = true;

  auto operator()(Input const& input) const -> Input {
    auto maxValue = input.max();
    auto expArray = input.project([maxValue](Input::UnderlyingType v) { return expf(v - maxValue); });
    auto sum = expArray.accumulate(0, [](Input::UnderlyingType lhs, Input::UnderlyingType rhs) { return lhs + rhs; });
    return expArray.transform([sum](Input::UnderlyingType v) { return RoundFloat<>()(v / sum); });
  }

  auto derive(Input const& input) const -> Input {
    auto aux = operator()(input);
    return aux * (Input::unit() - aux);
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
template <concepts::DeepLinearMatrixType Input, concepts::DeepLinearMatrixType KernelType, typename ConvType>
struct DeepConvolutionFunction {
  static_assert(Input::size() == KernelType::size(),
                "Cannot compute deep convolutions on input and kernel of different depths");

private:
  template <typename T = ConvType> using ConvResultType = typename T::ConvolutionResultType;

public:
  static constexpr auto isDeepConvolutionFunction = true;

  auto operator()(Input const& in, KernelType const& kernel) const {

    static constexpr auto rezLineSize = ConvResultType<>::size();
    static constexpr auto rezColumnSize = ConvResultType<>::total_size() / rezLineSize;

    LinearArray<typename Input::UnderlyingType, rezLineSize, rezColumnSize> result {};
    for (auto idx = 0; idx < Input::size(); ++idx) {
      result += static_cast<ConvType const*>(this)->convolve(in[idx], kernel[idx]);
    }
    return result;
  }

  template <typename T = ConvType>
  auto derive(Input const& in, typename T::ConvolutionResultType const& nextLayerGradient) const {
    return static_cast<ConvType const*>(this)->deriveConvolve(in, nextLayerGradient);
  }

  template <typename T = ConvType>
  auto fullyConvolve(typename T::ConvolutionResultType const& nextLayerGradient, KernelType const& kernel) const {
    return static_cast<ConvType const*>(this)->paddedConvolve(nextLayerGradient, kernel);
  }
};

template <concepts::DeepLinearMatrixType Input, typename PoolType> struct PoolingFunction {
private:
  template <typename T = PoolType> using ResultingPoolType = typename T::ResultingPoolType;

public:
  static constexpr auto isPoolingFunction = true;

  auto operator()(Input const& in) const {
    constexpr auto rezLineDepth = Input::size();
    constexpr auto rezLineSize = ResultingPoolType<>::size();
    constexpr auto rezColSize = ResultingPoolType<>::InnerLinearArray::size();

    LinearArray<typename Input::UnderlyingType, rezLineDepth, rezLineSize, rezColSize> result {};
    for (auto idx = 0; idx < Input::size(); ++idx) {
      result[idx] = static_cast<PoolType const*>(this)->pool(in[idx]);
    }
    return result;
  }

  template <typename T = PoolType, Size rezLineSize = T::ResultingPoolType::size(),
            Size rezColSize = T::ResultingPoolType::InnerLinearArray::size()>
  auto
  derive(Input const& in,
         LinearArray<typename Input::UnderlyingType, Input::size(), rezLineSize, rezColSize> const& gradient) const {
    Input result {};
    for (auto idx = 0; idx < in.size(); ++idx) {
      result[idx] = static_cast<PoolType const*>(this)->derivedPool(in[idx], gradient[idx]);
    }
    return result;
  }
};
} // namespace impl

template <concepts::DeepLinearMatrixType Input, concepts::DeepLinearMatrixType KernelType>
struct SimpleDeepConvolutionFunction :
    impl::DeepConvolutionFunction<Input, KernelType, SimpleDeepConvolutionFunction<Input, KernelType>> {
  using ConvolutionResultType =
      decltype(std::declval<typename Input::InnerLinearArray>().convolve(typename KernelType::InnerLinearArray {}));

  auto convolve(typename Input::InnerLinearArray const& in, typename KernelType::InnerLinearArray const& kernel) const {
    return in.convolve(kernel);
  }

  auto deriveConvolve(Input const& in, ConvolutionResultType const& gradient) const {
    KernelType result {};
    for (auto idx = 0; idx < in.size(); ++idx) {
      result[idx] = in[idx].convolve(gradient);
    }
    return result;
  }

  auto paddedConvolve(ConvolutionResultType const& gradient, KernelType const& kernel) const {
    Input result {};

    constexpr auto linePad = (KernelType::InnerLinearArray::size() + 1) / 2;
    constexpr auto colPad = (KernelType::InnerLinearArray::InnerLinearArray::size() + 1) / 2;
    auto paddedIn = gradient.template pad<linePad, colPad>();
    for (auto idx = 0; idx < kernel.size(); ++idx) {
      result[idx] = paddedIn.convolve(kernel[idx]);
    }
    return result;
  }
};

template <Size stride, concepts::DeepLinearMatrixType Input, concepts::DeepLinearMatrixType KernelType>
struct StridedDeepConvolutionFunction :
    impl::DeepConvolutionFunction<Input, KernelType, StridedDeepConvolutionFunction<stride, Input, KernelType>> {
  using ConvolutionResultType =
      decltype(std::declval<typename Input::InnerLinearArray>().template stridedConvolve<stride>(
          typename KernelType::InnerLinearArray {}));

  auto convolve(typename Input::InnerLinearArray const& in, typename KernelType::InnerLinearArray const& kernel) const {
    return in.template stridedConvolve<stride>(kernel);
  }

  auto deriveConvolve(Input const& in, ConvolutionResultType const& gradient) const {
    KernelType result {};
    auto dilatedGradient = gradient.template dilate<stride - 1>();
    for (auto idx = 0; idx < in.size(); ++idx) {
      result[idx] = in[idx].convolve(dilatedGradient);
    }
    return result;
  }

  auto paddedConvolve(ConvolutionResultType const& gradient, KernelType const& kernel) const {
    constexpr auto linePad = (KernelType::InnerLinearArray::size() + 1) / 2;
    constexpr auto colPad = (KernelType::InnerLinearArray::InnerLinearArray::size() + 1) / 2;
    auto remodeledIn = gradient.template dilate<stride - 1>().template pad<linePad, colPad>();
    Input result {};
    for (auto idx = 0; idx < kernel.size(); ++idx) {
      result[idx] = remodeledIn.convolve(kernel[idx]);
    }
    return result;
  }
};

namespace impl {
template <concepts::DeepLinearMatrixType Input, typename PoolDim, typename StrideDim> struct MaxPoolFunction :
    impl::PoolingFunction<Input, MaxPoolFunction<Input, PoolDim, StrideDim>> {

  static constexpr auto predicate = [](typename Input::UnderlyingType lhs, typename Input::UnderlyingType rhs) {
    return lhs < rhs;
  };

  using ResultingPoolType =
      decltype(std::declval<typename Input::InnerLinearArray>()
                   .template pool<PoolDim::size(), PoolDim::size(), StrideDim::size()>(decltype(predicate) {}));

  auto pool(typename Input::InnerLinearArray const& in) const {
    return in.template pool<PoolDim::size(), PoolDim::size(), StrideDim::size()>(predicate);
  }

  auto derivedPool(typename Input::InnerLinearArray const& in, ResultingPoolType const& gradient) const {
    constexpr auto inLines = Input::InnerLinearArray::size();
    constexpr auto inCols = Input::InnerLinearArray::InnerLinearArray::size();

    typename Input::InnerLinearArray result {};
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

    for (auto lIdx = 0, gLIdx = 0; lIdx < inLines && gLIdx < gradient.size(); lIdx += StrideDim::size(), ++gLIdx) {
      for (auto cIdx = 0, gColIdx = 0; cIdx < inCols && gColIdx < gradient[0].size();
           cIdx += StrideDim::size(), ++gColIdx) {
        localSearch(lIdx, cIdx, gLIdx, gColIdx);
      }
    }
    return result;
  }
};
} // namespace impl

template <concepts::DeepLinearMatrixType Input, Size poolSize, Size stride> struct MaxPoolFunction :
    impl::MaxPoolFunction<Input, gabe::nn::impl::Dimension<poolSize>, gabe::nn::impl::Dimension<stride>> {};

template <typename = void> struct ReluFunction {};
template <> struct ReluFunction<void> {
  static constexpr auto isActivationFunction = true;

  template <typename Input> auto operator()(Input&& in) {
    return ReluFunction<std::remove_cvref_t<Input>> {}(std::forward<Input>(in));
  }

  template <typename Input> auto derive(Input&& in) {
    return ReluFunction<std::remove_cvref_t<Input>> {}.derive(std::forward<Input>(in));
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
template <typename Input> constexpr SigmoidFunction<Input> sigmoid;
} // namespace func
} // namespace gabe::utils::math
