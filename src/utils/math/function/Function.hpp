//
// Created by stefan on 2/14/24.
//

#pragma once

#include "utils/concepts/Concepts.hpp"
#include <cmath>

namespace gabe::utils::math {

template <typename = void> struct SigmoidFunction {};

template <std::floating_point InputType> struct SigmoidFunction<InputType> {
  static constexpr auto isActivationFunction = true;
  auto operator()(InputType const& input) const -> double {
    return static_cast<InputType>(1) / (static_cast<InputType>(1) + std::exp(-input));
  }
  auto derive(InputType const& input) const -> double {
    auto aux = operator()(input);
    return aux * (static_cast<InputType>(1) - aux);
  }
};

template <> struct SigmoidFunction<void> {
  static constexpr auto isActivationFunction = true;
  static constexpr auto isTransparent = true;

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


template <typename InputType = void> struct MeanSquaredErrorFunction {};

template <> struct MeanSquaredErrorFunction<void> {
  static constexpr auto isCostFunction = true;

  template <typename InputType> auto operator()(InputType&& in, InputType&& target) const {
    return MeanSquaredErrorFunction<std::remove_cvref_t<InputType>>()(std::forward<InputType>(in),
                                                                      std::forward<InputType>(target));
  }

  template <typename InputType> auto derive(InputType&& in, InputType&& target) const {
    return MeanSquaredErrorFunction<std::remove_cvref_t<InputType>>().derive(std::forward<InputType>(in),
                                                                             std::forward<InputType>(target));
  }
};

template <concepts::IntegralType InputType> struct MeanSquaredErrorFunction<InputType> {
  static constexpr auto isCostFunction = true;

  auto operator()(InputType const& in, InputType const& target) const { return (target - in) * (target - in) / 2; }

  auto derive(InputType const& in, InputType const& target) const { return (in - target); }
};

template <concepts::LinearArrayType InputType> struct MeanSquaredErrorFunction<InputType> {
  static constexpr auto isCostFunction = true;

  auto operator()(InputType const& in, InputType const& target) const {
    auto divider = [](typename InputType::UnderlyingType const& val) {
      return val / static_cast<typename InputType::UnderlyingType>(2);
    };
    return ((target - in) * (target - in)).project(divider);
  }

  auto derive(InputType const& in, InputType const& target) const { return (in - target); }
};


namespace func {
template <typename InputType> constexpr SigmoidFunction<InputType> sigmoid;
} // namespace func
} // namespace gabe::utils::math
