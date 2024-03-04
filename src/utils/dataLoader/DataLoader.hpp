//
// Created by stefan on 3/4/24.
//

#pragma once

#include "types.hpp"
#include "utils/concepts/Concepts.hpp"
#include <bitset>
#include <fstream>

namespace gabe::utils::dataLoader {

namespace impl {
template <concepts::LinearArrayType R> auto loadMNISTData(std::ifstream& dataStream, uint32 expectedImageCount)
    -> std::vector<R> {
  auto* data = new char[4];
  dataStream.read(data, 4);
  uint32 magicNumber = ((uint8) data[3] | (uint8) data[2] << 8 | (uint8) data[1] << 16 | (uint8) data[0] << 24);
  assert(magicNumber == 2051 && "Magic number for mnist train data should be 2051");

  dataStream.read(data, 4);
  uint32 imageCount = ((uint8) data[3] | (uint8) data[2] << 8 | (uint8) data[1] << 16 | (uint8) data[0] << 24);
  assert(imageCount == expectedImageCount && "The image count for mnist train data should be 60000");

  dataStream.read(data, 4);
  uint32 rowCount = ((uint8) data[3] | (uint8) data[2] << 8 | (uint8) data[1] << 16 | (uint8) data[0] << 24);
  assert(rowCount == 28 && "The number of pixel rows for mnist train data should be 28");

  dataStream.read(data, 4);
  uint32 colCount = ((uint8) data[3] | (uint8) data[2] << 8 | (uint8) data[1] << 16 | (uint8) data[0] << 24);
  assert(colCount == 28 && "The number of pixel columns for mnist train data should be 28");
  delete[] data;

  std::vector<R> rez {};

  auto* pixelData = new char[28 * 28];
  for (auto dataIdx = 0; dataIdx < imageCount; ++dataIdx) {
    dataStream.read(pixelData, 28 * 28);
    std::array<typename R::UnderlyingType, 28 * 28> arr {};
    for (auto idx = 0; idx < 28 * 28; ++idx) {
      arr[idx] = static_cast<typename R::UnderlyingType>((uint8) pixelData[idx]);
    }
    rez.emplace_back(arr);
  }
  delete[] pixelData;

  return rez;
}
} // namespace impl

enum class MNISTLoad { TEST, TEST_LABEL, TRAIN, TRAIN_LABEL };

template <concepts::LinearArrayType R> auto loadMNIST(std::string const& filePath, MNISTLoad loadFlag)
    -> std::vector<R> {
  std::basic_ifstream<uint8> in(filePath, std::ios::binary);

  switch (loadFlag) {
    case MNISTLoad::TRAIN: return impl::loadMNISTData<R>(in, 60000);
    case MNISTLoad::TEST: return impl::loadMNISTData<R>(in, 10000);
  }
  return {};
}
} // namespace gabe::utils::dataLoader
