//
// Created by stefan on 2/8/24.
//

#pragma once

#include <array>
#include <cassert>
#include <types.hpp>

namespace gabe::utils::math {

template <typename DataType, Size first_size, Size... remaining_sizes> class LinearArray {
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

  auto operator+(LinearArray const& other) const -> LinearArray {
    LinearArray result;
    result._data = this->_data;
    for (int idx = 0; idx < first_size; ++idx) {
      result._data[idx] = result._data[idx] + other._data[idx];
    }
    return result;
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

private:
  std::array<InnerLinearArray, first_size> _data {};
};

template <typename DataType, Size size> class LinearArray<DataType, size> {
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

  auto begin() -> Iterator { return Iterator(this, 0); }
  auto end() -> Iterator { return Iterator(this, size); }

  template <typename N> auto operator[](N idx) -> DataType& {
    assert(std::is_signed_v<N> && idx >= 0 && idx < size && "Requested index out of range");
    return _data[idx];
  }

  template <typename N> auto operator[](N idx) const -> DataType const& {
    assert(std::is_signed_v<N> && idx >= 0 && idx < size && "Requested index out of range");
    return _data[idx];
  }

  auto operator+(LinearArray const& other) const -> LinearArray {
    LinearArray result;
    result._data = this->_data;
    for (int idx = 0; idx < size; ++idx) {
      result._data[idx] = result._data[idx] + other._data[idx];
    }
    return result;
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
} // namespace gabe::utils::math
