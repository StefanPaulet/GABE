//
// Created by stefan on 2/9/24.
//
#pragma once

#include <iostream>
#include <types.hpp>

namespace gabe::utils::math {

template <typename DataType, Size first_size, Size... remaining_sizes> class LinearArray;

namespace linearArray::impl {

template <typename> struct IsLinearArray : std::false_type {};
template <typename T, Size fs, Size... rs> struct IsLinearArray<LinearArray<T, fs, rs...>> : std::true_type {};

template <typename> struct IsMalformedLinearArray : std::false_type {};
template <typename T, Size s> struct IsMalformedLinearArray<LinearArray<T, s>> :
    std::bool_constant<IsLinearArray<T>::value> {};

template <Size...> struct SizePack {};

template <typename, Size> struct PackPush {};
template <Size... rs, Size s> struct PackPush<SizePack<rs...>, s> {
  using type = SizePack<s, rs...>;
};

template <typename> struct GetSizePackOfMLA {
  using type = SizePack<>;
  using inner_type = void;
};

template <typename T, Size s> struct GetSizePackOfMLA<LinearArray<T, s>> {
  using type = std::conditional_t<IsMalformedLinearArray<LinearArray<T, s>>::value,
                                  typename PackPush<typename GetSizePackOfMLA<T>::type, s>::type, SizePack<s>>;

  using inner_type =
      std::conditional_t<IsMalformedLinearArray<LinearArray<T, s>>::value, typename GetSizePackOfMLA<T>::inner_type, T>;
};

template <typename T, Size s1, Size s2, Size... sn> struct GetSizePackOfMLA<LinearArray<T, s1, s2, sn...>> {
  using type = SizePack<s1, s2, sn...>;
  using inner_type = T;
};


template <typename, typename> struct MakeLA {};
template <typename T, Size... s> struct MakeLA<T, SizePack<s...>> {
  using type = LinearArray<T, s...>;
};

template <typename T> struct TransformMLAtoLA {
  using inner_type_MLA = typename GetSizePackOfMLA<T>::inner_type;
  using size_pack_MLA = typename GetSizePackOfMLA<T>::type;
  using type = typename MakeLA<inner_type_MLA, size_pack_MLA>::type;
};
} // namespace linearArray::impl
} // namespace gabe::utils::math
