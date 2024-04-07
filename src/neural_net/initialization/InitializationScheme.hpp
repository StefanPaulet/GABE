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

template <typename = void> struct HeInitialization {};

template <> struct HeInitialization<void> {
  template <typename T, typename... Params> auto operator()(T&& in, Params... params) {
    return HeInitialization<std::remove_cvref_t<T>> {}(std::forward<T>(in), params...);
  }
};

template <gabe::utils::concepts::DeepKernelType KernelArray> struct HeInitialization<KernelArray> :
    InitializationScheme<HeInitialization<KernelArray>> {
  auto initialize(KernelArray& in, std::random_device& rd, Size depth) const {
    using KA = typename KernelArray::InnerLinearArray;
    std::normal_distribution<typename KA::UnderlyingType> distribution(
        0, 1.0 / std::sqrt(depth * KA::total_size() / KA::size()));
    auto transformer = [&distribution, &rd](typename KA::UnderlyingType) { return distribution(rd); };
    in.transform(transformer);
  }
};

template <gabe::utils::concepts::LinearMatrixType Matrix> struct HeInitialization<Matrix> :
    InitializationScheme<HeInitialization<Matrix>> {
  auto initialize(Matrix& in, std::random_device& rd) const {
    std::normal_distribution<typename Matrix::UnderlyingType> distribution(
        0, 1.0 / std::sqrt(in.total_size() / in.size()));
    auto transformer = [&distribution, &rd](typename Matrix::UnderlyingType) { return distribution(rd); };
    in.transform(transformer);
  }
};
} // namespace gabe::nn
