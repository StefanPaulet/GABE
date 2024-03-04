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
template <concepts::LinearArrayType R> auto loadMNISTData(FILE* dataStream, uint32 expectedImageCount)
    -> std::vector<R> {
  std::array<unsigned char, 4> data;
  fread(data.data(), 1, 4, dataStream);

  uint32 magicNumber = ((uint32) data[3] | (uint32) data[2] << 8 | (uint32) data[1] << 16 | (uint32) data[0] << 24);
  assert(magicNumber == 2051 && "Magic number for mnist train data should be 2051");

  fread(data.data(), 1, 4, dataStream);
  uint32 imageCount = ((uint32) data[3] | (uint32) data[2] << 8 | (uint32) data[1] << 16 | (uint32) data[0] << 24);
  assert(imageCount == expectedImageCount && "The image count for mnist train data should be 60000");

  fread(data.data(), 1, 4, dataStream);
  uint32 rowCount = ((uint32) data[3] | (uint32) data[2] << 8 | (uint32) data[1] << 16 | (uint32) data[0] << 24);
  assert(rowCount == 28 && "The number of pixel rows for mnist train data should be 28");

  fread(data.data(), 1, 4, dataStream);
  uint32 colCount = ((uint32) data[3] | (uint32) data[2] << 8 | (uint32) data[1] << 16 | (uint32) data[0] << 24);
  assert(colCount == 28 && "The number of pixel columns for mnist train data should be 28");

  std::vector<R> rez {};

  std::array<char, 28 * 28> pixelData;
  for (auto dataIdx = 0; dataIdx < imageCount; ++dataIdx) {
    fread(pixelData.data(), 1, 28 * 28, dataStream);
    std::array<typename R::UnderlyingType, 28 * 28> arr {};
    for (auto idx = 0; idx < 28 * 28; ++idx) {
      arr[idx] = static_cast<typename R::UnderlyingType>((uint8) pixelData[idx]);
    }
    rez.emplace_back(arr);
  }

  return rez;
}
} // namespace impl

enum class MNISTLoad { TEST, TEST_LABEL, TRAIN, TRAIN_LABEL };

template <concepts::LinearArrayType R> auto loadMNIST(std::string const& filePath, MNISTLoad loadFlag)
    -> std::vector<R> {
  FILE* in = fopen(filePath.c_str(), "rb");

  switch (loadFlag) {
    case MNISTLoad::TRAIN: return impl::loadMNISTData<R>(in, 60000);
    case MNISTLoad::TEST: return impl::loadMNISTData<R>(in, 10000);
  }
  return {};
}
} // namespace gabe::utils::dataLoader
