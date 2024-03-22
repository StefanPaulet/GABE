//
// Created by stefan on 2/8/24.
//

#pragma once

#include "../predicates/Predicates.hpp"
#include "LinearArrayTraits.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <numeric>
#include <ostream>

namespace gabe::utils::math {

namespace linearArray::impl {
template <typename DataType, Size s> class LinearArrayContainer {
  static_assert(s != 0, "Size of linear array must not be 0");

public:
  static constexpr auto first_size = s;

  LinearArrayContainer() = default;
  LinearArrayContainer(LinearArrayContainer const&) = default;
  LinearArrayContainer(LinearArrayContainer&&) noexcept = default;
  explicit LinearArrayContainer(std::array<DataType, s> const& data) : _data(data) {}
  auto operator=(LinearArrayContainer const&) -> LinearArrayContainer& = default;

  constexpr auto& data() { return _data; }
  constexpr auto const& data() const { return _data; }
  static constexpr auto size() { return s; }

private:
  std::array<DataType, s> _data;
};

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

  auto& operator+=(LinearArrayGenericOps const& rhs) {
    *static_cast<D*>(this) = *static_cast<D*>(this) + *static_cast<D const*>(&rhs);
    return *this;
  }

  auto& operator-=(LinearArrayGenericOps const& rhs) {
    *static_cast<D*>(this) = *static_cast<D*>(this) - *static_cast<D const*>(&rhs);
    return *this;
  }

  auto& operator*=(LinearArrayGenericOps const& rhs) {
    *static_cast<D*>(this) = *static_cast<D*>(this) * *static_cast<D const*>(&rhs);
    return *this;
  }

  auto& operator/=(LinearArrayGenericOps const& rhs) {
    *static_cast<D*>(this) = *static_cast<D*>(this) / *static_cast<D const*>(&rhs);
    return *this;
  }

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
    return *static_cast<D*>(this);
  }

  template <typename T> auto project(T&& transformer) const -> D {
    auto rez = *static_cast<D const*>(this);
    rez.transform(std::forward<T>(transformer));
    return rez;
  }

  template <typename R, typename A, typename T = D> auto accumulate(R initialValue, A&& accumulator) const ->
      typename T::UnderlyingType {
    static_assert(std::is_convertible_v<R, typename D::UnderlyingType>);

    typename D::UnderlyingType rez(initialValue);

    for (auto const& e : *static_cast<D const*>(this)) {
      rez = accumulator(rez, e.accumulate(initialValue, std::forward<A>(accumulator)));
    }
    return rez;
  }

  template <typename P> auto maximize(LinearArrayGenericOps const& other, P&& predicate) const -> D {
    auto otherArr = static_cast<D const*>(&other)->data();
    auto rez = *static_cast<D const*>(this);
    auto& rezData = rez.data();
    for (auto idx = 0; idx < otherArr.size(); ++idx) {
      rezData[idx] = rezData[idx].maximize(otherArr[idx], std::forward<P>(predicate));
    }
    return rez;
  }

  auto max() const {
    auto currMax = (*static_cast<D const*>(this)->data().begin()).max();
    for (auto const& e : static_cast<D const*>(this)->data()) {
      currMax = std::max(currMax, e.max());
    }
    return currMax;
  }

  auto min() const {
    auto currMin = (*static_cast<D const*>(this)->data().begin()).min();
    for (auto const& e : static_cast<D const*>(this)->data()) {
      currMin = std::min(currMin, e.min());
    }
    return currMin;
  }

  static constexpr auto total_size() { return D::size() * D::InnerLinearArray::total_size(); }

  auto begin() { return static_cast<D*>(this)->data().begin(); }
  auto end() { return static_cast<D*>(this)->data().end(); }

  auto begin() const { return static_cast<D const*>(this)->data().begin(); }
  auto end() const { return static_cast<D const*>(this)->data().end(); }

  constexpr auto flatten() const {
    LinearArray<typename D::UnderlyingType, total_size()> rez;
    auto data = static_cast<D const*>(this)->data();
    for (auto idx = 0; idx < data.size(); ++idx) {
      auto flatData = data[idx].flatten();
      std::memcpy(rez.data().data() + idx * flatData.size(), flatData.data().data(),
                  flatData.size() * sizeof(typename D::UnderlyingType));
    }
    return rez;
  }

  template <typename T = D> static constexpr auto unit(typename T::UnderlyingType const& unit = 1) {
    D result {};
    auto row_value = result.data()[0].unit() * unit;
    for (auto& e : result.data()) {
      e = row_value;
    }
    return result;
  }
};

template <typename FD> auto operator+(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = static_cast<FD const*>(&lhs)->data();
  auto const& rhsArr = static_cast<FD const*>(&rhs)->data();

  FD result;
  std::ranges::copy(lhsArr, result.begin());
  std::ranges::for_each(result.data(), [&rhsArr, i = 0](auto& e) mutable { e = e + rhsArr[i++]; });
  return result;
}

template <typename FD> auto operator-(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = static_cast<FD const*>(&lhs)->data();
  auto const& rhsArr = static_cast<FD const*>(&rhs)->data();

  FD result;
  std::ranges::copy(lhsArr, result.begin());
  std::ranges::for_each(result.data(), [&rhsArr, i = 0](auto& e) mutable { e = e - rhsArr[i++]; });
  return result;
}

template <typename FD> auto operator*(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = static_cast<FD const*>(&lhs)->data();
  auto const& rhsArr = static_cast<FD const*>(&rhs)->data();

  FD result;
  std::ranges::copy(lhsArr, result.begin());
  std::ranges::for_each(result.data(), [&rhsArr, i = 0](auto& e) mutable { e = e * rhsArr[i++]; });
  return result;
}

template <typename FD> auto operator/(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = static_cast<FD const*>(&lhs)->data();
  auto const& rhsArr = static_cast<FD const*>(&rhs)->data();

  FD result;
  std::ranges::copy(lhsArr, result.begin());
  std::ranges::for_each(result.data(), [&rhsArr, i = 0](auto& e) mutable { e = e / rhsArr[i++]; });
  return result;
}

template <typename FD> auto operator==(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> bool {
  return Equals<>()(static_cast<FD const*>(&lhs)->data(), static_cast<FD const*>(&rhs)->data());
}

template <typename FD> auto operator<<(std::ostream& out, LinearArrayGenericOps<FD> const& obj) -> std::ostream& {
  out << "[";
  auto const& data = static_cast<FD const*>(&obj)->data();
  for (auto iter = data.begin(); iter != data.end(); ++iter) {
    if (iter != data.begin()) {
      out << ", ";
    }
    out << *iter;
  }
  out << "]";
  return out;
}

template <typename FD, concepts::IntegralType V> auto operator+(LinearArrayGenericOps<FD> const& arr, V value) -> FD {
  return static_cast<FD const*>(&arr)->project([value]<typename T>(T&& val) { return std::forward<T>(val) + value; });
}

template <typename FD, concepts::IntegralType V> auto operator-(LinearArrayGenericOps<FD> const& arr, V value) -> FD {
  return static_cast<FD const*>(&arr)->project([value]<typename T>(T&& val) { return std::forward<T>(val) - value; });
}

template <typename FD, concepts::IntegralType V> auto operator*(LinearArrayGenericOps<FD> const& arr, V value) -> FD {
  return static_cast<FD const*>(&arr)->project([value]<typename T>(T&& val) { return std::forward<T>(val) * value; });
}

template <typename FD, concepts::IntegralType V> auto operator/(LinearArrayGenericOps<FD> const& arr, V value) -> FD {
  return static_cast<FD const*>(&arr)->project([value]<typename T>(T&& val) { return std::forward<T>(val) / value; });
}

template <typename FD, concepts::IntegralType V> auto operator+(V value, LinearArrayGenericOps<FD> const& arr) -> FD {
  return static_cast<FD const*>(&arr)->project([value]<typename T>(T&& val) { return value + std::forward<T>(val); });
}

template <typename FD, concepts::IntegralType V> auto operator-(V value, LinearArrayGenericOps<FD> const& arr) -> FD {
  return static_cast<FD const*>(&arr)->project([value]<typename T>(T&& val) { return value - std::forward<T>(val); });
}

template <typename FD, concepts::IntegralType V> auto operator*(V value, LinearArrayGenericOps<FD> const& arr) -> FD {
  return static_cast<FD const*>(&arr)->project([value]<typename T>(T&& val) { return value * std::forward<T>(val); });
}

template <typename FD, concepts::IntegralType V> auto operator/(V value, LinearArrayGenericOps<FD> const& arr) -> FD {
  return static_cast<FD const*>(&arr)->project([value]<typename T>(T&& val) { return value / std::forward<T>(val); });
}

} // namespace linearArray::impl


template <typename DataType, Size first_size, Size... remaining_sizes> class LinearArray :
    public linearArray::impl::LinearArrayContainer<LinearArray<DataType, remaining_sizes...>, first_size>,
    public linearArray::impl::LinearArrayGenericOps<LinearArray<DataType, first_size, remaining_sizes...>> {

public:
  using InnerLinearArray = LinearArray<DataType, remaining_sizes...>;
  using linearArray::impl::LinearArrayContainer<InnerLinearArray, first_size>::data;
  using UnderlyingType = DataType;

private:
  using LinearArrayContainer = linearArray::impl::LinearArrayContainer<InnerLinearArray, first_size>;

public:
  using LinearArrayContainer::first_size;
  using LinearArrayContainer::LinearArrayContainer;

  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  LinearArray(LinearArray&&) noexcept = default;

  auto operator=(LinearArray const& other) -> LinearArray& = default;
  auto operator=(LinearArray&& other) noexcept -> LinearArray& = default;
};

template <typename DataType, Size line_size, Size col_size> class LinearArray<DataType, line_size, col_size> :
    public linearArray::impl::LinearArrayContainer<LinearArray<DataType, col_size>, line_size>,
    public linearArray::impl::LinearArrayGenericOps<LinearArray<DataType, line_size, col_size>> {
  static_assert(std::is_constructible_v<DataType, int>, "LinearMatrix is intended to work with arithmetic types");
  using LinearArrayContainer = linearArray::impl::LinearArrayContainer<LinearArray<DataType, col_size>, line_size>;

public:
  using InnerLinearArray = LinearArray<DataType, col_size>;
  using linearArray::impl::LinearArrayContainer<LinearArray<DataType, col_size>, line_size>::data;
  using UnderlyingType = DataType;
  using LinearArrayContainer::LinearArrayContainer;
  using linearArray::impl::LinearArrayGenericOps<LinearArray<DataType, line_size, col_size>>::unit;

  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  LinearArray(LinearArray&&) noexcept = default;

  template <Size array_size> explicit LinearArray(std::array<DataType, array_size> const& arr) {
    static_assert(col_size * line_size == array_size, "Non-matching size from one dimensional array to matrix");

    for (Size lineIdx = 0; lineIdx < line_size; ++lineIdx) {
      std::memcpy(data()[lineIdx].data().data(), arr.data() + lineIdx * col_size, col_size * sizeof(DataType));
    }
  }

  auto operator=(LinearArray const& other) -> LinearArray& = default;
  auto operator=(LinearArray&& other) noexcept -> LinearArray& = default;

  static constexpr auto identity(DataType const& unit = 1) -> LinearArray {
    static_assert(line_size == col_size && "Unit matrix exists only on square matrices");

    auto result = LinearArray();
    for (Size lineIdx = 0; lineIdx < line_size; ++lineIdx) {
      result[lineIdx][lineIdx] = unit;
    }
    return result;
  }

  static constexpr auto nul(DataType const& nulVal = 0) -> LinearArray {
    auto result = LinearArray();
    for (auto& l : result.data()) {
      for (auto& e : l.data()) {
        e = nulVal;
      }
    }
    return result;
  }

  auto transpose() const -> LinearArray<DataType, col_size, line_size> {
    auto rez = LinearArray<DataType, col_size, line_size>();
    for (int lineIdx = 0; lineIdx < line_size; ++lineIdx) {
      for (int colIdx = 0; colIdx < col_size; ++colIdx) {
        rez[colIdx][lineIdx] = data()[lineIdx][colIdx];
      }
    }
    return rez;
  }

  template <Size rez_col_size> auto product(LinearArray<DataType, col_size, rez_col_size> const& rhs) const
      -> LinearArray<DataType, line_size, rez_col_size> {
    auto rez = LinearArray<DataType, line_size, rez_col_size>();
    for (int lineIdx = 0; lineIdx < line_size; ++lineIdx) {
      for (int colIdx = 0; colIdx < col_size; ++colIdx) {
        for (int cellIdx = 0; cellIdx < rez_col_size; ++cellIdx) {
          rez[lineIdx][cellIdx] += data()[lineIdx][colIdx] * rhs[colIdx][cellIdx];
        }
      }
    }
    return rez;
  }

  template <Size conv_line_size, Size conv_col_size>
  auto convolve(LinearArray<DataType, conv_line_size, conv_col_size> const& kernel) const
      -> LinearArray<DataType, line_size - conv_line_size + 1, col_size - conv_col_size + 1> {
    auto rez = LinearArray<DataType, line_size - conv_line_size + 1, col_size - conv_col_size + 1>();

    auto add = [&](int lineIdx, int colIdx) {
      for (int convLineIdx = 0; convLineIdx < conv_line_size; ++convLineIdx) {
        for (int convColIdx = 0; convColIdx < conv_col_size; ++convColIdx) {
          rez[lineIdx][colIdx] += data()[lineIdx + convLineIdx][colIdx + convColIdx] * kernel[convLineIdx][convColIdx];
        }
      }
    };
    for (int lineIdx = 0; lineIdx < line_size - conv_line_size + 1; ++lineIdx) {
      for (int colIdx = 0; colIdx < col_size - conv_col_size + 1; ++colIdx) {
        add(lineIdx, colIdx);
      }
    }
    return rez;
  }

  template <Size stride, Size conv_line_size, Size conv_col_size>
  auto stridedConvolve(LinearArray<DataType, conv_line_size, conv_col_size> const& kernel) const
      -> LinearArray<DataType, ((line_size - conv_line_size) / stride + 1), ((col_size - conv_col_size) / stride + 1)> {
    auto rez =
        LinearArray<DataType, ((line_size - conv_line_size) / stride + 1), ((col_size - conv_col_size) / stride + 1)>();

    auto add = [&](Size rezLIdx, Size rezCIdx, Size lineIdx, Size colIdx) {
      for (Size convLineIdx = 0; convLineIdx < conv_line_size; ++convLineIdx) {
        for (Size convColIdx = 0; convColIdx < conv_col_size; ++convColIdx) {
          rez[rezLIdx][rezCIdx] += data()[lineIdx + convLineIdx][colIdx + convColIdx] * kernel[convLineIdx][convColIdx];
        }
      }
    };
    for (Size lineIdx = 0, rezLine = 0; lineIdx < line_size - conv_line_size + 1; lineIdx += stride, ++rezLine) {
      for (Size colIdx = 0, rezCol = 0; colIdx < col_size - conv_col_size + 1; colIdx += stride, ++rezCol) {
        add(rezLine, rezCol, lineIdx, colIdx);
      }
    }
    return rez;
  }
};

template <typename DataType, Size line_size, Size col_size> using LinearMatrix =
    LinearArray<DataType, line_size, col_size>;

template <typename DataType, Size size> using SquareLinearMatrix = LinearMatrix<DataType, size, size>;

template <typename DataType, Size colSize> using LinearColumnArray = LinearMatrix<DataType, colSize, 1>;
template <typename DataType, Size lineSize> using LinearLineArray = LinearMatrix<DataType, 1, lineSize>;
template <typename DataType, Size size> using Kernel = SquareLinearMatrix<DataType, size>;

template <typename DataType, Size size> class LinearArray<DataType, size> :
    public linearArray::impl::LinearArrayContainer<DataType, size>,
    public linearArray::impl::LinearArrayGenericOps<LinearArray<DataType, size>> {
  static_assert(std::is_constructible_v<DataType, int>, "LinearArray is intended to work with arithmetic types");
  using LinearArrayContainer = linearArray::impl::LinearArrayContainer<DataType, size>;

public:
  using linearArray::impl::LinearArrayContainer<DataType, size>::data;
  using UnderlyingType = DataType;
  using LinearArrayContainer::LinearArrayContainer;

  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  LinearArray(LinearArray&&) noexcept = default;

  auto dot(LinearArray const& other) const -> DataType {
    DataType sum = 0;
    for (auto const& e : (*this) * other) {
      sum += e;
    }
    return sum;
  }

  auto max() const -> DataType { return std::ranges::max(data()); }
  auto min() const -> DataType { return std::ranges::min(data()); }

  auto operator=(LinearArray const& other) -> LinearArray& = default;
  auto operator=(LinearArray&& other) noexcept -> LinearArray& = default;

  template <typename T> auto& transform(T&& transformer) {
    for (auto& e : data()) {
      e = std::forward<T>(transformer)(e);
    }
    return *this;
  }

  template <typename T> auto project(T&& transformer) const -> LinearArray {
    auto rez = *this;
    for (auto& e : rez.data()) {
      e = std::forward<T>(transformer)(e);
    }
    return rez;
  }

  template <typename V, typename A> auto accumulate(V initialValue, A&& accumulator) const -> UnderlyingType {
    return std::accumulate(data().begin(), data().end(), static_cast<UnderlyingType>(initialValue), accumulator);
  }

  static constexpr auto total_size() { return size; }

  template <typename P> auto maximize(LinearArray const& other, P&& predicate) const -> LinearArray {
    LinearArray rez {};
    for (auto idx = 0; idx < size; ++idx) {
      if (std::forward<P>(predicate)(data()[idx], other[idx])) {
        rez[idx] = data()[idx];
      } else {
        rez[idx] = other[idx];
      }
    }
    return rez;
  }

  constexpr auto flatten() -> LinearArray { return LinearArray {*this}; }

  static constexpr auto unit(DataType const& unit = 1) -> LinearArray {
    auto result = LinearArray();
    for (auto& e : result.data()) {
      e = unit;
    }
    return result;
  }

  static constexpr auto nul(DataType const& nulVal = 0) -> LinearArray {
    auto result = LinearArray();
    for (auto& e : result.data()) {
      e = nulVal;
    }
    return result;
  }
};

namespace linearArray {
template <
    typename... Types,
    typename R = typename impl::TransformMLAtoLA<LinearArray<std::common_type_t<Types...>, sizeof...(Types)>>::type>
auto larray(Types&&... types) noexcept -> R {
  return R {{static_cast<std::common_type_t<Types...>>(std::forward<Types>(types))...}};
}
} // namespace linearArray
} // namespace gabe::utils::math
