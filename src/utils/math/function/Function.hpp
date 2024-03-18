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

template <concepts::LinearMatrixType InputType, concepts::LinearMatrixType KernelType, Size = 0, typename = void>
struct ConvolutionFunction {
  auto operator()(InputType const& in, KernelType const& kernel) { return in.convolve(kernel); }
};

template <concepts::LinearMatrixType InputType, concepts::LinearMatrixType KernelType, Size stride>
struct ConvolutionFunction<InputType, KernelType, stride, std::enable_if_t<stride != 0, void>> {
  auto operator()(InputType const& in, KernelType const& kernel) { return in.template stridedConvolve<stride>(kernel); }
};


namespace func {
template <typename InputType> constexpr SigmoidFunction<InputType> sigmoid;
} // namespace func
} // namespace gabe::utils::math
