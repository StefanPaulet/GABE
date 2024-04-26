//
// Created by stefan on 3/11/24.
//

#pragma once

#include <algorithm>
#include <ranges>
#include <vector>

namespace gabe::utils::data {

template <concepts::LinearArrayType DataPointType> struct ImageDataPoint {
  DataPointType data {};
  typename DataPointType::UnderlyingType label {};
};

template <concepts::LinearArrayType T> class ImageDataSet {

public:
  ImageDataSet() = default;
  ImageDataSet(ImageDataSet const&) = default;
  ImageDataSet(ImageDataSet&&) noexcept = default;
  explicit ImageDataSet(std::vector<ImageDataPoint<T>> const& data) : _data(data) {}

  auto normalize() {

    auto min = _data[0].data;
    auto max = _data[0].data;
    for (auto const& e : _data) {
      min = min.maximize(e.data, std::less<typename T::UnderlyingType> {});
      max = max.maximize(e.data, std::greater<typename T::UnderlyingType> {});
    }
    auto diff = max - min;

    auto normalizer = [min, diff](ImageDataPoint<T>& arr) { arr.data = (arr.data - min) / diff; };
    std::ranges::for_each(_data, normalizer);
  }

  auto const& data() const { return _data; }
  auto& data() { return _data; }

private:
  std::vector<ImageDataPoint<T>> _data {};
};


template <concepts::DeepLinearMatrixType ImageDataPoint> struct YoloDataPoint {
  ImageDataPoint data {};
  std::vector<gabe::utils::math::LinearArray<typename ImageDataPoint::UnderlyingType, 5, 1>> labels {};
};

template <concepts::DeepLinearMatrixType T> class YoloDataSet {
private:
  using ImageDataPoint = YoloDataPoint<T>;

public:
  YoloDataSet() = default;
  YoloDataSet(YoloDataSet const&) = default;
  YoloDataSet(YoloDataSet&&) noexcept = default;
  explicit YoloDataSet(std::vector<ImageDataPoint> const& data) : _data(data) {}

  auto normalize() {
    auto divideBy255 = [](typename T::UnderlyingType value) {
      return value / static_cast<typename T::UnderlyingType>(255);
    };
    auto transformer = [&divideBy255](ImageDataPoint& el) { el.data.transform(divideBy255); };
    std::ranges::for_each(_data, transformer);
  }

  auto const& data() const { return _data; }
  auto& data() { return _data; }

private:
  std::vector<ImageDataPoint> _data {};
};

} // namespace gabe::utils::data
