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

    auto min = _data[0].data;
    auto max = _data[0].data;
    for (auto const& e : _data) {
      min = min.maximize(e.data, std::less<typename T::UnderlyingType> {});
      max = max.maximize(e.data, std::greater<typename T::UnderlyingType> {});
    }
    auto diff = max - min;

    auto normalizer = [min, diff](DataPoint<T>& arr) { arr.data = (arr.data - min) / diff; };
    std::ranges::for_each(_data, normalizer);
  }

  auto const& data() const { return _data; }
  auto& data() { return _data; }

private:
  std::vector<DataPoint<T>> _data {};
};

} // namespace gabe::utils::data
