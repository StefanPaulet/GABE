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
#include <thread>

namespace gabe::utils::math {

namespace linearArray::impl {
template <typename DataType, Size s> class LinearArrayContainer {
  static_assert(s != 0, "Size of linear array must not be 0");

public:
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

  auto serialize(FILE* outFile) const {
    auto arr = static_cast<D const*>(this)->data();
    for (auto const& e : arr) {
      e.serialize(outFile);
    }
  }

  static auto deserialize(FILE* inFile) -> D {
    D result {};
    for (auto idx = 0; idx < result.size(); ++idx) {
      result[idx] = D::InnerLinearArray::deserialize(inFile);
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
  std::ranges::for_each(result.data(), [&rhsArr, i = 0](auto& e) mutable {
    if constexpr (concepts::IntegralType<std::remove_cvref_t<decltype(rhsArr[i])>>) {
      if (HighPrecisionEquals<> {}(rhsArr[i], static_cast<typename FD::UnderlyingType>(0))) {
        assert(false && "Cannot divide by 0");
      }
    }
    e = e / rhsArr[i++];
  });
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
  using LinearArrayContainer::LinearArrayContainer;

  LinearArray() = default;
  LinearArray(LinearArray const&) = default;
  LinearArray(LinearArray&&) noexcept = default;

  template <Size array_size> explicit LinearArray(std::array<DataType, array_size> const& arr) {
    static_assert(LinearArray::total_size() == array_size, "Non-matching size from one dimensional array to matrix");

    for (Size lineIdx = 0; lineIdx < first_size; ++lineIdx) {
      std::array<DataType, LinearArray::total_size() / first_size> slicedArr {};
      std::memcpy(slicedArr.data(), arr.data() + first_size * lineIdx,
                  LinearArray::total_size() / first_size * sizeof(DataType));
      data()[lineIdx] = InnerLinearArray {slicedArr};
    }
  }

  template <Size array_size> explicit LinearArray(LinearArray<DataType, array_size> const& lArr) :
      LinearArray(lArr.data()) {}

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

  template <Size array_size> explicit LinearArray(LinearArray<DataType, array_size> const& lArr) :
      LinearArray(lArr.data()) {}

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

    auto localProd = [&rez, &rhs, this](Size lStIdx, Size cStIdx, Size lEnIdx, Size cEnIdx) {
      for (Size lineIdx = lStIdx; lineIdx < lEnIdx; ++lineIdx) {
        for (Size colIdx = cStIdx; colIdx < cEnIdx; ++colIdx) {
          for (Size cellIdx = 0; cellIdx < col_size; ++cellIdx) {
            rez[lineIdx][colIdx] += data()[lineIdx][cellIdx] * rhs[cellIdx][colIdx];
          }
        }
      }
    };

    if constexpr (line_size > 3 && rez_col_size > 7 && line_size + rez_col_size >= 4096) {
      constexpr auto threadCount = 32;
      auto constexpr equivDistLine = line_size / threadCount * 2;
      auto constexpr equivDistCol = rez_col_size / threadCount;

      std::array<std::pair<Size, Size>, threadCount> constexpr lDir {{{equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4}}};

      std::array<std::pair<Size, Size>, threadCount> constexpr cDir {{{equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4},
                                                                      {equivDistCol * 4, equivDistCol * 5},
                                                                      {equivDistCol * 5, equivDistCol * 6},
                                                                      {equivDistCol * 6, equivDistCol * 7},
                                                                      {equivDistCol * 7, equivDistCol * 8},
                                                                      {equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4},
                                                                      {equivDistCol * 4, equivDistCol * 5},
                                                                      {equivDistCol * 5, equivDistCol * 6},
                                                                      {equivDistCol * 6, equivDistCol * 7},
                                                                      {equivDistCol * 7, equivDistCol * 8},
                                                                      {equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4},
                                                                      {equivDistCol * 4, equivDistCol * 5},
                                                                      {equivDistCol * 5, equivDistCol * 6},
                                                                      {equivDistCol * 6, equivDistCol * 7},
                                                                      {equivDistCol * 7, equivDistCol * 8},
                                                                      {equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4},
                                                                      {equivDistCol * 4, equivDistCol * 5},
                                                                      {equivDistCol * 5, equivDistCol * 6},
                                                                      {equivDistCol * 6, equivDistCol * 7},
                                                                      {equivDistCol * 7, equivDistCol * 8}}};
      auto closure = [&localProd, &lDir, &cDir](Size idx) {
        return [idx, localProd, &lDir, &cDir] {
          return localProd(lDir[idx].first, cDir[idx].first, lDir[idx].second, cDir[idx].second);
        };
      };
      {
        std::array<std::jthread, threadCount> threadArr {std::jthread {closure(0)}, std::jthread {closure(1)},
                                                         std::jthread {closure(2)}, std::jthread {closure(3)},
                                                         std::jthread {closure(4)}, std::jthread {closure(5)},
                                                         std::jthread {closure(6)}, std::jthread {closure(7)},
                                                         std::jthread {closure(8)}, std::jthread {closure(9)},
                                                         std::jthread {closure(10)}, std::jthread {closure(11)},
                                                         std::jthread {closure(12)}, std::jthread {closure(13)},
                                                         std::jthread {closure(14)}, std::jthread {closure(15)},
                                                         std::jthread {closure(16)}, std::jthread {closure(17)},
                                                         std::jthread {closure(18)}, std::jthread {closure(19)},
                                                         std::jthread {closure(20)}, std::jthread {closure(21)},
                                                         std::jthread {closure(22)}, std::jthread {closure(23)},
                                                         std::jthread {closure(24)}, std::jthread {closure(25)},
                                                         std::jthread {closure(26)}, std::jthread {closure(27)},
                                                         std::jthread {closure(28)}, std::jthread {closure(29)},
                                                         std::jthread {closure(30)}, std::jthread {closure(31)}};
      }
    } else if constexpr (line_size > 3 && rez_col_size > 3 && line_size + rez_col_size >= 1024) {
      constexpr auto threadCount = 16;
      auto constexpr equivDistLine = line_size / threadCount;
      auto constexpr equivDistCol = rez_col_size / threadCount;

      std::array<std::pair<Size, Size>, threadCount> constexpr lDir {{{equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 2, equivDistLine * 3},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4},
                                                                      {equivDistLine * 3, equivDistLine * 4}}};

      std::array<std::pair<Size, Size>, threadCount> constexpr cDir {{{equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4},
                                                                      {equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4},
                                                                      {equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4},
                                                                      {equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4}}};
      auto closure = [&localProd, &lDir, &cDir](Size idx) {
        return [idx, localProd, &lDir, &cDir] {
          return localProd(lDir[idx].first, cDir[idx].first, lDir[idx].second, cDir[idx].second);
        };
      };
      {
        std::array<std::jthread, threadCount> threadArr {std::jthread {closure(0)}, std::jthread {closure(1)},
                                                         std::jthread {closure(2)}, std::jthread {closure(3)},
                                                         std::jthread {closure(4)}, std::jthread {closure(5)},
                                                         std::jthread {closure(6)}, std::jthread {closure(7)},
                                                         std::jthread {closure(8)}, std::jthread {closure(9)},
                                                         std::jthread {closure(10)}, std::jthread {closure(11)},
                                                         std::jthread {closure(12)}, std::jthread {closure(13)},
                                                         std::jthread {closure(14)}, std::jthread {closure(15)}};
      }
    } else if constexpr (line_size > 1 && rez_col_size > 3 && line_size + rez_col_size >= 256) {
      constexpr auto threadCount = 8;
      auto constexpr equivDistLine = line_size / threadCount * 2;
      auto constexpr equivDistCol = rez_col_size / threadCount;

      std::array<std::pair<Size, Size>, threadCount> constexpr lDir {{{equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2}}};

      std::array<std::pair<Size, Size>, threadCount> constexpr cDir {{{equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4},
                                                                      {equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 2, equivDistCol * 3},
                                                                      {equivDistCol * 3, equivDistCol * 4}}};
      auto closure = [&localProd, &lDir, &cDir](Size idx) {
        return [idx, localProd, &lDir, &cDir] {
          return localProd(lDir[idx].first, cDir[idx].first, lDir[idx].second, cDir[idx].second);
        };
      };
      {
        std::array<std::jthread, threadCount> threadArr {std::jthread {closure(0)}, std::jthread {closure(1)},
                                                         std::jthread {closure(2)}, std::jthread {closure(3)},
                                                         std::jthread {closure(4)}, std::jthread {closure(5)},
                                                         std::jthread {closure(6)}, std::jthread {closure(7)}};
      }
    } else if constexpr (line_size > 1 && rez_col_size > 1 && line_size + rez_col_size >= 64) {
      constexpr auto threadCount = 4;
      auto constexpr equivDistLine = line_size / threadCount;
      auto constexpr equivDistCol = rez_col_size / threadCount;

      std::array<std::pair<Size, Size>, threadCount> constexpr lDir {{{equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 0, equivDistLine * 1},
                                                                      {equivDistLine * 1, equivDistLine * 2},
                                                                      {equivDistLine * 1, equivDistLine * 2}}};

      std::array<std::pair<Size, Size>, threadCount> constexpr cDir {{{equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2},
                                                                      {equivDistCol * 0, equivDistCol * 1},
                                                                      {equivDistCol * 1, equivDistCol * 2}}};
      auto closure = [&localProd, &lDir, &cDir](Size idx) {
        return [idx, localProd, &lDir, &cDir] {
          return localProd(lDir[idx].first, cDir[idx].first, lDir[idx].second, cDir[idx].second);
        };
      };
      {
        std::array<std::jthread, threadCount> threadArr {std::jthread {closure(0)}, std::jthread {closure(1)},
                                                         std::jthread {closure(2)}, std::jthread {closure(3)}};
      }
    } else {
      localProd(0, 0, line_size, rez_col_size);
    }

    return rez;
  }

  template <Size conv_line_size, Size conv_col_size, Size stride = 1,
            Size rlSize = (line_size - conv_line_size) / stride + 1,
            Size rcSize = (col_size - conv_col_size) / stride + 1>
  auto convolve(LinearArray<DataType, conv_line_size, conv_col_size> const& kernel) const
      -> LinearArray<DataType, rlSize, rcSize> {
    auto rez = LinearArray<DataType, rlSize, rcSize> {};

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

  template <Size stride, Size cls, Size ccs> auto stridedConvolve(LinearArray<DataType, cls, ccs> const& kernel) const {
    return convolve<cls, ccs, stride>(kernel);
  }

  auto flip() const -> LinearArray {
    static_assert(line_size == col_size && "Only square matrices can be flipped");
    LinearArray rez {};
    for (auto lIdx = 0; lIdx < this->size(); ++lIdx) {
      for (auto cIdx = 0; cIdx < this->size(); ++cIdx) {
        rez[lIdx][cIdx] = (*this)[this->size() - lIdx - 1][this->size() - cIdx - 1];
      }
    }
    return rez;
  }

  template <Size dilation, Size rlSize = line_size + dilation*(line_size - 1),
            Size rcSize = col_size + dilation*(col_size - 1)>
  auto dilate() const -> LinearArray<DataType, rlSize, rcSize> {
    LinearArray<DataType, rlSize, rcSize> rez {};
    for (Size rezLIdx = 0, lIdx = 0; lIdx < line_size; ++lIdx, rezLIdx += dilation + 1) {
      for (Size rezCIdx = 0, cIdx = 0; cIdx < col_size; ++cIdx, rezCIdx += dilation + 1) {
        rez[rezLIdx][rezCIdx] = data()[lIdx][cIdx];
      }
    }
    return rez;
  }

  template <Size pool_line_size, Size pool_col_size, typename PoolingPredicate, Size stride = 1,
            Size rlSize = (line_size - pool_line_size) / stride + 1,
            Size rcSize = (col_size - pool_col_size) / stride + 1>
  auto __pool(PoolingPredicate&& predicate) const -> LinearArray<DataType, rlSize, rcSize> {
    LinearArray<DataType, rlSize, rcSize> rez {};

    auto localPool = [&](Size rezLIdx, Size rezCIdx, Size lineIdx, Size colIdx) {
      DataType currentMax = data()[lineIdx][colIdx];
      for (Size lIdx = 0; lIdx < pool_line_size; ++lIdx) {
        for (Size cIdx = 0; cIdx < pool_col_size; ++cIdx) {
          currentMax =
              std::max(currentMax, data()[lineIdx + lIdx][colIdx + cIdx], std::forward<PoolingPredicate>(predicate));
        }
      }
      rez[rezLIdx][rezCIdx] = currentMax;
    };
    for (Size lineIdx = 0, rezLine = 0; lineIdx < line_size - pool_line_size + 1; lineIdx += stride, ++rezLine) {
      for (Size colIdx = 0, rezCol = 0; colIdx < col_size - pool_col_size + 1; colIdx += stride, ++rezCol) {
        localPool(rezLine, rezCol, lineIdx, colIdx);
      }
    }
    return rez;
  }

  template <Size pool_line_size, Size pool_col_size, Size stride, typename PoolingPredicate>
  auto pool(PoolingPredicate&& predicate) const {
    return __pool<pool_line_size, pool_col_size, PoolingPredicate, stride>(std::forward<PoolingPredicate>(predicate));
  }

  template <Size line_pad_size, Size col_pad_size, Size rlSize = line_size + 2 * line_pad_size,
            Size rcSize = col_size + 2 * col_pad_size>
  auto pad() const {
    LinearArray<DataType, rlSize, rcSize> result {};
    for (auto lIdx = line_pad_size; lIdx < rlSize - line_pad_size; ++lIdx) {
      std::memcpy(result[lIdx].data().data() + col_pad_size, data()[lIdx - line_pad_size].data().data(),
                  col_size * sizeof(DataType));
    }
    return result;
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
    return std::accumulate(data().begin(), data().end(), static_cast<UnderlyingType>(initialValue),
                           std::forward<A>(accumulator));
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

  auto serialize(FILE* outFile) const { fwrite(data().data(), sizeof(DataType), size, outFile); }
  static auto deserialize(FILE* inFile) -> LinearArray {
    LinearArray result;
    fread(result.data().data(), sizeof(DataType), size, inFile);
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
