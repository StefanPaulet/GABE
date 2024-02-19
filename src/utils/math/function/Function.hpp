//
// Created by stefan on 2/14/24.
//

#pragma once

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


namespace func {
template <typename InputType> constexpr SigmoidFunction<InputType> sigmoid;
} // namespace func
} // namespace gabe::utils::math
