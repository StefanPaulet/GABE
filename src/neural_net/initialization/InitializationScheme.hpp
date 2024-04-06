//
// Created by stefan on 4/6/24.
//

#pragma once

#include "utils/concepts/Concepts.hpp"
#include <random>

namespace gabe::nn {
template <typename D> struct InitializationScheme {
  template <typename In, typename... Params> auto operator()(In& in, Params... params) const {
    std::random_device rd {};
    static_cast<D const*>(this)->initialize(in, rd, params...);
  }
};

template <Size d = 0, typename = void> struct HeInitialization {};

template <Size depth, gabe::utils::concepts::DeepKernelType KernelArray> struct HeInitialization<depth, KernelArray> :
    InitializationScheme<HeInitialization<depth, KernelArray>> {
  auto initialize(KernelArray& in, std::random_device& rd) const {
    using KA = typename KernelArray::InnerLinearArray;
    std::normal_distribution<typename KA::UnderlyingType> distribution(
        0, 1.0 / std::sqrt(depth * KA::total_size() / KA::size()));
    auto transformer = [&distribution, &rd](typename KA::UnderlyingType) { return distribution(rd); };
    in.transform(transformer);
  }
};

template <Size d> struct HeInitialization<d, void> {
  template <typename T> auto operator()(T&& in) {
    return HeInitialization<d, std::remove_cvref_t<T>> {}(std::forward<T>(in));
  }
};
} // namespace gabe::nn
