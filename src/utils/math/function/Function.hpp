//
// Created by stefan on 2/14/24.
//

#pragma once

#include <cmath>

namespace gabe::utils::math {

template <typename D, typename InputType, typename OutputType> class ActivationFunction {
  static_assert(std::is_constructible_v<InputType, int> && std::is_constructible_v<OutputType, int>,
                "Function is intended to work with integer arithmetic types");

public:
  auto operator()(InputType const& input) const -> OutputType { return static_cast<D const*>(this)->operator()(input); }
  auto derive(InputType const& input) const -> OutputType { return static_cast<D const*>(this)->derive(input); }
};

template <typename InputType> class SigmoidFunction :
    public ActivationFunction<SigmoidFunction<InputType>, InputType, double> {
public:
  auto operator()(InputType const& input) const -> double { return 1.0 / (1.0 + std::exp(-input)); }
  auto derive(InputType const& input) const -> double {
    auto aux = operator()(input);
    return aux * (1.0 - aux);
  }
};

template <typename InputType> class IdentityFunction :
    public ActivationFunction<IdentityFunction<InputType>, InputType, InputType> {
public:
  auto operator()(InputType const& input) const -> InputType { return input; }
  auto derive(InputType const& input) const -> InputType { return static_cast<InputType>(1); }
};

namespace func {
template <typename InputType> constexpr SigmoidFunction<InputType> sigmoid;
} // namespace func
} // namespace gabe::utils::math
