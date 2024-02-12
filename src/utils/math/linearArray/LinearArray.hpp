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

  template <typename T> auto project(T&& transformer) -> D {
    auto rez = *static_cast<D*>(this);
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

  FD result = lhsArr;
  Size idx;
  auto backInsert = [&result, &idx](auto const& entry) { result[idx++] -= entry; };
  idx = 0;
  std::ranges::for_each(rhsArr, backInsert);
  return result;
}

template <typename FD> auto operator*(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> FD {
  auto const& lhsArr = *static_cast<FD const*>(&lhs);
  auto const& rhsArr = *static_cast<FD const*>(&rhs);

  FD result = FD::unit();
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

  FD result = lhsArr;
  Size idx;
  auto backInsert = [&result, &idx](auto const& entry) {
    assert(entry != 0 && "Cannot divide by 0");
    result[idx++] /= entry;
  };
  idx = 0;
  std::ranges::for_each(rhsArr, backInsert);
  return result;
}

template <typename FD> auto operator==(LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs)
    -> bool {
  return static_cast<FD const*>(&lhs)->data() == static_cast<FD const*>(&rhs)->data();
}

template <typename FD> auto operator<<(std::ostream& out, LinearArrayGenericOps<FD> const& obj) -> std::ostream& {
  out << "[";
  auto const* self = static_cast<FD const*>(&obj);
  for (auto iter = self->data().begin(); iter != self->data().end(); ++iter) {
    if (iter != self->data().begin()) {
      out << ", ";
    }
    out << *iter;
  }
  out << "]";
  return out;
}

//TODO create base class with _data for access with 2 specializations

template <typename DataType, Size first_size, Size... remaining_sizes> class LinearArray :
    public LinearArrayGenericOps<LinearArray<DataType, first_size, remaining_sizes...>> {
  static_assert(first_size != 0, "Size of linear array must not be 0");

public:
  using InnerLinearArray = LinearArray<DataType, remaining_sizes...>;

  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  LinearArray(LinearArray&&) noexcept = default;
  explicit LinearArray(std::array<InnerLinearArray, first_size> const& data) : _data(data) {}

  auto& data() { return _data; }
  auto const& data() const { return _data; }

private:
  std::array<InnerLinearArray, first_size> _data {};
};

template <typename DataType, Size line_size, Size col_size> class LinearArray<DataType, line_size, col_size> :
    public LinearArrayGenericOps<LinearArray<DataType, line_size, col_size>> {
  static_assert(line_size != 0 && col_size != 0, "Size of linear matrix must not be 0");

public:
  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  LinearArray(LinearArray&&) noexcept = default;
  explicit LinearArray(std::array<LinearArray<DataType, col_size>, line_size> const& other) {
    for (int lineIdx = 0; lineIdx < line_size; ++lineIdx) {
      _data[lineIdx].data() = other[lineIdx].data();
    }
  }

  auto& data() { return _data; }
  auto const& data() const { return _data; }

  auto transpose() const -> LinearArray<DataType, col_size, line_size> {
    auto rez = LinearArray<DataType, col_size, line_size>();
    for (int lineIdx = 0; lineIdx < line_size; ++lineIdx) {
      for (int colIdx = 0; colIdx < col_size; ++colIdx) {
        rez[colIdx][lineIdx] = this->_data[lineIdx][colIdx];
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
          rez[lineIdx][cellIdx] += this->_data[lineIdx][colIdx] * rhs[colIdx][cellIdx];
        }
      }
    }
    return rez;
  }

  template <Size conv_line_size, Size conv_col_size>
  auto convolve(LinearArray<DataType, conv_line_size, conv_col_size> const& kernel) const
      -> LinearArray<DataType, line_size - conv_line_size + 1, col_size - conv_col_size + 1> {
    auto rez = LinearArray<DataType, line_size - conv_line_size + 1, col_size - conv_col_size + 1>();

    for (int lineIdx = 0; lineIdx < line_size - conv_line_size + 1; ++lineIdx) {
      for (int colIdx = 0; colIdx < col_size - conv_col_size + 1; ++colIdx) {
        for (int convLineIdx = 0; convLineIdx < conv_line_size; ++convLineIdx) {
          for (int convColIdx = 0; convColIdx < conv_col_size; ++convColIdx) {
            rez[lineIdx][colIdx] +=
                this->_data[lineIdx + convLineIdx][colIdx + convColIdx] * kernel[convLineIdx][convColIdx];
          }
        }
      }
    }
    return rez;
  }

private:
  std::array<LinearArray<DataType, col_size>, line_size> _data;
};

template <typename DataType, Size line_size, Size col_size> using LinearMatrix =
    LinearArray<DataType, line_size, col_size>;

template <typename DataType, Size size> using SquareLinearMatrix = LinearMatrix<DataType, size, size>;

template <typename DataType, Size size> class LinearArray<DataType, size> :
    public LinearArrayGenericOps<LinearArray<DataType, size>> {
  static_assert(size != 0, "Size of linear array must not be 0");

public:
  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  LinearArray(LinearArray&&) noexcept = default;
  explicit LinearArray(std::array<DataType, size> const& array) : _data(array) {}

  auto& data() { return _data; }
  auto const& data() const { return _data; }

  auto dot(LinearArray const& other) const -> DataType {
    DataType sum = 0;
    for (auto const& e : (*this) * other) {
      sum += e;
    }
    return sum;
  }

  auto operator=(LinearArray const& other) -> LinearArray& = default;

  template <typename T> auto& transform(T&& transformer) {
    for (auto& e : _data) {
      e = std::forward<T>(transformer)(e);
    }
    return *this;
  }

  template <typename T> auto project(T&& transformer) -> LinearArray {
    auto rez = *this;
    for (auto& e : rez._data) {
      e = std::forward<T>(transformer)(e);
    }
    return rez;
  }

  static constexpr auto unit(DataType const& unit = 1) -> LinearArray {
    auto result = LinearArray();
    for (auto& e : result._data) {
      e = unit;
    }
    return result;
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
