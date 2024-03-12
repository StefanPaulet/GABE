//
// Created by stefan on 3/11/24.
//

#pragma once

namespace gabe::utils::data {

template <concepts::LinearArrayType DataPointType> struct DataPoint {
  DataPointType data {};
  typename DataPointType::UnderlyingType label {};
};

template <concepts::LinearArrayType T> class DataSet {

public:
  DataSet() = default;
  DataSet(DataSet const&) = default;
  DataSet(DataSet&&) noexcept = default;
  explicit DataSet(std::vector<DataPoint<T>> const& data) : _data(data) {}

  auto normalize() {
    auto normalizer = [](DataPoint<T>& arr) {
      auto min = arr.data.min();
      auto max = arr.data.max();
      auto transformer = [min, max](typename T::UnderlyingType value) { return (value - min) / (max - min); };
      arr.data.transform(transformer);
    };

    std::ranges::for_each(_data, normalizer);
  }

  auto const& data() const { return _data; }
  auto& data() { return _data; }

private:
  std::vector<DataPoint<T>> _data {};
};

} // namespace gabe::utils::data
