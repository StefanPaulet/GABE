//
// Created by stefan on 2/20/24.
//

#pragma once

#include <type_traits>
#include <types.hpp>

namespace gabe::nn {
namespace impl {
template <Size s> struct Dimension {
  static constexpr auto size = s;
};
template <template <typename...> typename LayerType, typename... Params> struct NDL {
  template <typename DataType> using Type = LayerType<DataType, Params...>;
};

template <typename> struct IsNDL : std::false_type {};
template <template <typename...> typename LayerType, typename... Params> struct IsNDL<NDL<LayerType, Params...>> :
    std::true_type {};

template <typename T, bool = IsNDL<T>::value> struct NDLType {};
template <typename T> struct NDLType<T, true> {
  template <typename DT> using Type = typename T::template Type<DT>;
};
template <typename T> struct NDLType<T, false> {
  template <typename> using Type = T;
};
} // namespace impl

template <Size s, template <typename...> typename L, typename... P> using SizedLayer =
    impl::NDL<L, P..., impl::Dimension<s>>;
} // namespace gabe::nn
