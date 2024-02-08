//
// Created by stefan on 2/8/24.
//

#pragma once

#include <array>
#include <cassert>
#include <algorithm>
#include <types.hpp>

namespace gabe::utils::math {

template <typename> class LinearArrayGenericOps {
public:
  template <typename FD> friend auto operator+ (
      LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs
  ) noexcept(noexcept(false)) -> FD;
};

template <typename FD> auto operator+ (
      LinearArrayGenericOps<FD> const& lhs, LinearArrayGenericOps<FD> const& rhs
) noexcept(noexcept(false)) -> FD {
  auto const& lhsArr = *static_cast<FD const*>(&lhs);
  auto const& rhsArr = *static_cast<FD const*>(&rhs);

  FD result;
  Size idx;
  auto backInsert = [&result, &idx](auto const& entry) {
    result[idx++] += entry;
  };
  idx = 0;
  std::for_each(lhsArr.begin(), lhsArr.end(), backInsert);
  idx = 0;
  std::for_each(rhsArr.begin(), rhsArr.end(), backInsert);
  return result;
}

template <typename DataType, Size first_size, Size... remaining_sizes> class LinearArray :
    public LinearArrayGenericOps<LinearArray<DataType, first_size, remaining_sizes...>> {
public:
  using InnerLinearArray = LinearArray<DataType, remaining_sizes...>;

  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  explicit LinearArray(std::array<InnerLinearArray, first_size> const& data) : _data(data) {}

  template <typename N> auto operator[](N idx) -> InnerLinearArray& {
    assert(std::is_signed_v<N> && idx >= 0 && idx < first_size && "Requested index out of range");
    return _data[idx];
  }

  template <typename N> auto operator[](N idx) const -> InnerLinearArray const& {
    assert(std::is_signed_v<N> && idx >= 0 && idx < first_size && "Requested index out of range");
    return _data[idx];
  }

  auto operator*(LinearArray const& other) const -> LinearArray {
    LinearArray result;
    result._data = this->_data;
    for (int idx = 0; idx < first_size; ++idx) {
      result._data[idx] = result._data[idx] * other._data[idx];
    }
    return result;
  }

  auto operator-(LinearArray const& other) const -> LinearArray {
    LinearArray result;
    result._data = this->_data;
    for (int idx = 0; idx < first_size; ++idx) {
      result._data[idx] = result._data[idx] - other._data[idx];
    }
    return result;
  }

  auto operator/(LinearArray const& other) const -> LinearArray {
    LinearArray result;
    result._data = this->_data;
    for (int idx = 0; idx < first_size; ++idx) {
      result._data[idx] = result._data[idx] / other._data[idx];
    }
    return result;
  }

  auto begin() { return _data.begin(); }
  auto end() { return _data.end(); }

  auto begin() const { return _data.begin(); }
  auto end() const { return _data.end(); }

private:
  std::array<InnerLinearArray, first_size> _data {};
};

template <typename DataType, Size size> class LinearArray<DataType, size> :
    public LinearArrayGenericOps<LinearArray<DataType, size>> {
public:
  LinearArray() = default;
  explicit LinearArray(std::array<DataType, size> const& array) : _data(array) {}

  class Iterator {
  public:
    Iterator(LinearArray* owner, int index) : _pOwner(owner), _index(index) {}

    auto operator*() const -> DataType& { return (*_pOwner)[_index]; }
    auto operator->() const -> DataType* { return &((*_pOwner)[_index]); }
    auto operator++() -> Iterator& {
      ++this->_index;
      return *this;
    }

    auto operator++(int) -> Iterator {
      Iterator result = *this;
      ++(*this);
      return result;
    }
    auto operator==(Iterator const& other) const -> bool = default;

  private:
    LinearArray* _pOwner;
    int _index {0};
  };

  auto begin() { return _data.begin(); }
  auto end() { return _data.end(); }

  auto begin() const { return _data.begin(); }
  auto end() const { return _data.end(); }

  template <typename N> auto operator[](N idx) -> DataType& {
    assert(idx >= 0 && idx < size && "Requested index out of range");
    return _data[idx];
  }

  template <typename N> auto operator[](N idx) const -> DataType const& {
    assert(std::is_signed_v<N> && idx >= 0 && idx < size && "Requested index out of range");
    return _data[idx];
  }

  auto operator*(LinearArray const& other) const -> LinearArray {
    LinearArray result;
    result._data = this->_data;
    for (int idx = 0; idx < size; ++idx) {
      result._data[idx] = result._data[idx] * other._data[idx];
    }
    return result;
  }

  auto operator-(LinearArray const& other) const -> LinearArray {
    LinearArray result;
    result._data = this->_data;
    for (int idx = 0; idx < size; ++idx) {
      result._data[idx] = result._data[idx] - other._data[idx];
    }
    return result;
  }

  auto operator/(LinearArray const& other) const -> LinearArray {
    LinearArray result;
    result._data = this->_data;
    for (int idx = 0; idx < size; ++idx) {
      result._data[idx] = result._data[idx] / other._data[idx];
    }
    return result;
  }

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
template <typename... Types> auto array(Types&&... types) noexcept -> LinearArray<std::common_type_t<Types...>, sizeof...(Types)> {
  return LinearArray<std::common_type_t<Types...>, sizeof...(Types)>{{static_cast<std::common_type_t<Types...>>(std::forward<Types>(types))...}};
}
} // namespace linearArray
} // namespace gabe::utils::math
