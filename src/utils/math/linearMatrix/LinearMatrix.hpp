//
// Created by stefan on 2/10/24.
//

#pragma once

#include "../linearArray/LinearArray.hpp"

namespace gabe::utils::math {
template <typename DataType, Size line_size, Size col_size> class LinearArray<DataType, line_size, col_size> :
    public LinearArrayGenericOps<LinearArray<DataType, line_size, col_size>> {
public:
  explicit LinearArray(std::array<std::array<DataType, col_size>, line_size> const& array) : _data(array) {}
  explicit LinearArray(std::array<LinearArray<DataType, col_size>, line_size> const& other) {
    for (int lineIdx = 0; lineIdx < line_size; ++lineIdx) {
      _data[lineIdx] = other[lineIdx].data();
    }
  }

  auto& data() { return _data; }
  auto const& data() const { return _data; }

  friend auto operator<<(std::ostream& out, LinearArray const& obj) -> std::ostream& {
    for (auto const& line : obj._data) {
      out << "[";
      for (auto iter = line.begin(); iter != line.end(); ++iter) {
        if (iter != line.begin()) {
          out << ", ";
        }
        out << *iter;
      }
      out << "]\n";
    }
    return out;
  }

  template <typename T> auto& transform(T&& transformer) {
    for (auto& line : _data) {
      for (auto& e : line) {
        e = std::forward<T>(transformer)(e);
      }
    }
    return *this;
  }

  auto transpose() const -> LinearArray<DataType, col_size, line_size> {
    auto rez = LinearArray<DataType, col_size, line_size>();
    for (int lineIdx = 0; lineIdx < line_size; ++lineIdx) {
      for (int colIdx = 0; colIdx < col_size; ++colIdx) {
        rez._data[colIdx][lineIdx] = this->_data[lineIdx][colIdx];
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
          rez._data[lineIdx][colIdx] += this->_data[lineIdx][cellIdx] * rhs._data[cellIdx][colIdx];
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
            rez._data[lineIdx][colIdx] +=
                this->_data[lineIdx + convLineIdx][colIdx + convColIdx] * kernel._data[convLineIdx][convColIdx];
          }
        }
      }
    }
    return rez;
  }

private:
  std::array<std::array<DataType, col_size>, line_size> _data;
};

template <typename DataType, Size line_size, Size col_size> using LinearMatrix =
    LinearArray<DataType, line_size, col_size>;
} // namespace gabe::utils::math