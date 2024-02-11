//
// Created by stefan on 2/8/24.
//

#pragma once

#include "LinearArrayTraits.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <ostream>

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

  template <typename FD> friend auto operator<<(std::ostream&, LinearArrayGenericOps<FD> const&) -> std::ostream&;

  template <typename T> auto& transform(T&& transformer) {
    for (auto& e : *static_cast<D*>(this)) {
      e.transform(std::forward<T>(transformer));
    }
    return *this;
  }

  template <typename T> auto project(T&& transformer) {
    auto rez = *this;
    rez.transform(std::forward<T>(transformer));
    return rez;
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

template <typename FD> auto operator<<(std::ostream& out, LinearArrayGenericOps<FD> const& obj) -> std::ostream& {
  out << "{\n";
  for (auto const& e : static_cast<FD const*>(&obj)->data()) {
    out << "\t{" << e << "}\n";
  }
  out << "}";
  return out;
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

  auto operator=(LinearArray const& other) -> LinearArray& = default;

  friend auto operator<<(std::ostream& out, LinearArray const& obj) -> std::ostream& {
    for (auto iter = obj._data.begin(); iter != obj._data.end(); ++iter) {
      if (iter != obj._data.begin()) {
        out << ", ";
      }
      out << *iter;
    }
    return out;
  }

  template <typename T> auto& transform(T&& transformer) {
    for (auto& e : _data) {
      e = std::forward<T>(transformer)(e);
    }
    return *this;
  }

private:
  std::array<DataType, size> _data {};
};

namespace linearArray {
template <
    typename... Types,
    typename R = typename impl::TransformMLAtoLA<LinearArray<std::common_type_t<Types...>, sizeof...(Types)>>::type>
auto larray(Types&&... types) noexcept -> R {
  return R{{static_cast<std::common_type_t<Types...>>(std::forward<Types>(types))...}};
}
} // namespace linearArray
} // namespace gabe::utils::math
