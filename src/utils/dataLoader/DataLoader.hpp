//
// Created by stefan on 3/4/24.
//

#pragma once

#include "types.hpp"
#include "utils/concepts/Concepts.hpp"
#include "utils/math/function/Function.hpp"
#include <bitset>
#include <fstream>
#include <sstream>

namespace gabe::utils::dataLoader {

template <concepts::LinearArrayType DataPointType> struct DataPoint {
  DataPointType data {};
  typename DataPointType::UnderlyingType label {};
};

namespace impl {
template <concepts::LinearArrayType R> auto loadMNISTData(FILE* dataStream, uint32 expectedImageCount)
    -> std::vector<R> {
  std::array<unsigned char, 4> data;
  fread(data.data(), 1, 4, dataStream);

  uint32 magicNumber = ((uint32) data[3] | (uint32) data[2] << 8 | (uint32) data[1] << 16 | (uint32) data[0] << 24);
  assert(magicNumber == 2051 && "Magic number for mnist data should be 2051");

  fread(data.data(), 1, 4, dataStream);
  uint32 imageCount = ((uint32) data[3] | (uint32) data[2] << 8 | (uint32) data[1] << 16 | (uint32) data[0] << 24);
  assert(imageCount == expectedImageCount && "The image count should be 60000 for train data/10000 for test data");

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

template <typename T> auto loadMNISTLabels(FILE* dataStream, uint32 expectedImageCount) -> std::vector<T> {
  std::array<unsigned char, 4> data;
  fread(data.data(), 1, 4, dataStream);

  uint32 magicNumber = ((uint32) data[3] | (uint32) data[2] << 8 | (uint32) data[1] << 16 | (uint32) data[0] << 24);
  assert(magicNumber == 2049 && "Magic number for mnist labels should be 2049");

  fread(data.data(), 1, 4, dataStream);
  uint32 imageCount = ((uint32) data[3] | (uint32) data[2] << 8 | (uint32) data[1] << 16 | (uint32) data[0] << 24);
  assert(imageCount == expectedImageCount && "TThe label count should be 60000 for train data/10000 for test data");

  std::vector<T> rez {};

  std::array<char, 1> labelData;
  for (auto dataIdx = 0; dataIdx < imageCount; ++dataIdx) {
    fread(labelData.data(), 1, 1, dataStream);
    rez.push_back(labelData[0]);
  }

  return rez;
}
} // namespace impl

template <typename T> using DataSet = std::vector<DataPoint<T>>;

enum class MNISTDataSet { TEST, TRAIN };

template <concepts::LinearArrayType R> auto loadMNIST(std::string const& filePath, MNISTDataSet loadFlag)
    -> DataSet<R> {
  FILE* imagesIn = fopen((filePath + "/images").c_str(), "rb");
  FILE* labelsIn = fopen((filePath + "/labels").c_str(), "rb");
  auto expectedImageCount = loadFlag == MNISTDataSet::TRAIN ? 60000 : 10000;

  auto labelsVector = impl::loadMNISTLabels<typename R::UnderlyingType>(labelsIn, expectedImageCount);
  auto dataVector = impl::loadMNISTData<R>(imagesIn, expectedImageCount);

  assert(labelsVector.size() == dataVector.size()
         && "The number of labels is not the same as the number of data points");

  auto rezVector = std::vector<DataPoint<R>> {};
  rezVector.reserve(labelsVector.size());
  for (auto idx = 0; idx < labelsVector.size(); ++idx) {
    rezVector.emplace_back(dataVector[idx], labelsVector[idx]);
  }

  fclose(imagesIn);
  fclose(labelsIn);
  return rezVector;
}

template <concepts::LinearArrayType R> auto loadDelimSeparatedFile(std::string const& filePath, char delim = ',')
    -> DataSet<R> {
  DataSet<R> rezVector;

  std::ifstream in {filePath};
  auto* linePtr = new char[1024];
  auto* wordPtr = new char[16];

  while (in.getline(linePtr, 1024)) {
    R data;

    std::string line {linePtr};
    auto lastDelimIdx = line.find_last_of(delim);
    auto label = utils::math::StringToIntegral<typename R::UnderlyingType>()(line.substr(lastDelimIdx + 1));

    std::stringstream lineString {line.substr(0, lastDelimIdx)};

    Size idx = 0;
    while (lineString.getline(wordPtr, 16, delim)) {
      std::string word {wordPtr};
      if (word.empty()) {
        continue;
      }
      data[idx++] = utils::math::StringToIntegral<typename R::UnderlyingType>()(word);
    }

    rezVector.emplace_back(data, label);
  }

  delete[] linePtr;
  delete[] wordPtr;

  return rezVector;
}
} // namespace gabe::utils::dataLoader
