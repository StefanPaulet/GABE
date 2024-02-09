//
// Created by stefan on 2/8/24.
//

#pragma once

#include <array>
#include <cassert>
#include <algorithm>
#include <types.hpp>

namespace gabe::utils::math {

template <typename D> class LinearArrayGenericOps {
public:
  template <typename FD>
  friend auto operator+(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs) -> FD;

  template <typename FD>
  friend auto operator-(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs) -> FD;

  template <typename FD>
  friend auto operator*(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs) -> FD;

  template <typename FD>
  friend auto operator/(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs) -> FD;

  template <typename FD> friend auto operator==(LinearArrayGenericOps<FD> const&, LinearArrayGenericOps<FD> const&)
      -> bool;

  template <typename N> auto& operator[](N idx) {
    auto& data = static_cast<D*>(this)->data();
    assert(idx >= 0 && idx < data.size() && "Requested index out of range");
    return data[idx];
  }

  template <typename N> auto const& operator[](N idx) const {
    auto& data = static_cast<D const*>(this)->data();
    assert(idx >= 0 && idx < data.size() && "Requested index out of range");
    return data[idx];
  }

  auto begin() { return static_cast<D*>(this)->data().begin(); }
  auto end() { return static_cast<D*>(this)->data().end(); }

  auto begin() const { return static_cast<D const*>(this)->data().begin(); }
  auto end() const { return static_cast<D const*>(this)->data().end(); }
};

template <typename FD> auto operator+(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = *static_cast<FD const*>(&lhs);
  auto const& rhsArr = *static_cast<FD const*>(&rhs);

  FD result;
  Size idx;
  auto backInsert = [&result, &idx](auto const& entry) { result[idx++] += entry; };
  idx = 0;
  std::ranges::for_each(lhsArr, backInsert);
  idx = 0;
  std::ranges::for_each(rhsArr, backInsert);
  return result;
}

template <typename FD> auto operator-(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = *static_cast<FD const*>(&lhs);
  auto const& rhsArr = *static_cast<FD const*>(&rhs);

  FD result;
  Size idx;
  auto backInsert = [&result, &idx](auto const& entry) { result[idx++] -= entry; };
  idx = 0;
  std::ranges::for_each(lhsArr, backInsert);
  idx = 0;
  std::ranges::for_each(rhsArr, backInsert);
  return result;
}

template <typename FD> auto operator*(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = *static_cast<FD const*>(&lhs);
  auto const& rhsArr = *static_cast<FD const*>(&rhs);

  FD result;
  Size idx;
  auto backInsert = [&result, &idx](auto const& entry) { result[idx++] *= entry; };
  idx = 0;
  std::ranges::for_each(lhsArr, backInsert);
  idx = 0;
  std::ranges::for_each(rhsArr, backInsert);
  return result;
}

template <typename FD> auto operator/(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = *static_cast<FD const*>(&lhs);
  auto const& rhsArr = *static_cast<FD const*>(&rhs);

  FD result;
  Size idx;
  auto backInsert = [&result, &idx](auto const& entry) { result[idx++] /= entry; };
  idx = 0;
  std::ranges::for_each(lhsArr, backInsert);
  idx = 0;
  std::ranges::for_each(rhsArr, backInsert);
  return result;
}

template <typename FD> auto operator==(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> bool {
  return static_cast<FD const*>(&lhs)->data() == static_cast<FD const*>(&rhs)->data();
}

template <typename DataType, Size first_size, Size... remaining_sizes> class LinearArray :
    public LinearArrayGenericOps<LinearArray<DataType, first_size, remaining_sizes...>> {
public:
  using InnerLinearArray = LinearArray<DataType, remaining_sizes...>;

  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  explicit LinearArray(std::array<InnerLinearArray, first_size> const& data) : _data(data) {}

  auto& data() { return _data; }
  auto const& data() const { return _data; }

private:
  std::array<InnerLinearArray, first_size> _data {};
};

template <typename DataType, Size size> class LinearArray<DataType, size> :
    public LinearArrayGenericOps<LinearArray<DataType, size>> {
public:
  LinearArray() = default;
  explicit LinearArray(std::array<DataType, size> const& array) : _data(array) {}

  auto& data() { return _data; }
  auto const& data() const { return _data; }

  auto dot(LinearArray const& other) const -> int {
    int sum = 0;
    for (auto const& e : (*this) * other) {
      sum += e;
    }
    return sum;
  }

private:
  std::array<DataType, size> _data {};
};

namespace linearArray {
template <typename> struct IsLinearArray : std::false_type {};
template <typename T, Size f, Size... s> struct IsLinearArray<LinearArray<T, f, s...>> : std::true_type {};

template <typename> struct IsMalformedLinearArray : std::false_type {};
template <typename T, Size f> struct IsMalformedLinearArray<LinearArray<T, f>> :
    std::bool_constant<IsLinearArray<T>::value> {};

template <Size... > struct SizePack {};

template <typename T, Size s> struct PackPush {};
template <Size ...p, Size s> struct PackPush<SizePack<p...>, s> {
  using type = SizePack<s, p...>;
};

namespace h1 {
template <typename> struct GetSizePackOfMLA {
  using type = SizePack<>;
  using inner_type = void;
};

template <typename T, Size s> struct GetSizePackOfMLA<LinearArray<T, s>> {
  using type = std::conditional_t<
      IsMalformedLinearArray<LinearArray<T, s>>::value,
      typename PackPush<typename GetSizePackOfMLA<T>::type, s>::type,
      SizePack<s>
  >;

  using inner_type = std::conditional_t<
      IsMalformedLinearArray<LinearArray<T, s>>::value,
      typename GetSizePackOfMLA<T>::inner_type,
      T
  >;
};
}

template <typename T> using GetSizePackOfMLA = typename h1::GetSizePackOfMLA<T>::type;
template <typename T> using GetInnerTypeOfMLA = typename h1::GetSizePackOfMLA<T>::inner_type;

namespace h2 {
template <typename, typename> struct ComposeLA {};
template <typename I, Size... s> struct ComposeLA<I, SizePack<s...>> {
  using type = LinearArray<I, s...>;
};
}

template <typename L, typename R> using ComposeLA = typename h2::ComposeLA<L, R>::type;

namespace h3 {
template <typename T> struct TransformMLAToLA {
  using pack_type = GetSizePackOfMLA<T>;
  using inner_type = GetInnerTypeOfMLA<T>;
  using resulted_type = ComposeLA<inner_type, pack_type>;
};
}

template <typename T> using MLAToLA = typename h3::TransformMLAToLA<T>::resulted_type;

template <
    typename... Types,
    typename R = MLAToLA<LinearArray<std::common_type_t<Types...>, sizeof...(Types)>>
> auto larray(Types&&... types) noexcept
    -> R {
  return R{{static_cast<std::common_type_t<Types...>>(std::forward<Types>(types))...}};
}
} // namespace linearArray
} // namespace gabe::utils::math
