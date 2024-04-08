//
// Created by stefan on 3/4/24.
//

#pragma once

#include "types.hpp"
#include "utils/concepts/Concepts.hpp"
#include "utils/data/Data.hpp"
#include "utils/math/function/Function.hpp"
#include <bitset>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <jpeglib.h>
#include <sstream>

namespace gabe::utils::data {

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

template <concepts::DeepLinearMatrixType R> auto loadJPEG(std::string const& filePath) -> R {
  R result {};

  FILE* in = fopen(filePath.c_str(), "rb");

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  jpeg_stdio_src(&cinfo, in);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  int width = cinfo.output_width;
  int height = cinfo.output_height;
  int numChannels = cinfo.num_components;
  assert(width == 640 && "Width of CS2 image should be 640");
  assert(height == 640 && "Height of CS2 image should be 640");
  assert(numChannels == 3 && "The number of channels of CS2 image should be 3");

  auto* imageBuffer = new unsigned char[width * height * numChannels];

  while (cinfo.output_scanline < cinfo.output_height) {
    unsigned char* row_pointer = &imageBuffer[cinfo.output_scanline * width * numChannels];
    jpeg_read_scanlines(&cinfo, &row_pointer, 1);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(in);

  for (Size channel = 0; channel < numChannels; ++channel) {
    for (Size lIdx = 0; lIdx < height; ++lIdx) {
      for (Size cIdx = 0; cIdx < width; ++cIdx) {
        result[channel][lIdx][cIdx] = imageBuffer[(width * lIdx + cIdx) * numChannels + channel];
      }
    }
  }
  delete[] imageBuffer;
  return result;
}

template <concepts::DeepLinearMatrixType K> auto loadCS2Labels(std::string const& filePath)
    -> std::vector<gabe::utils::math::LinearArray<typename K::UnderlyingType, 5, 1>> {
  std::vector<gabe::utils::math::LinearArray<typename K::UnderlyingType, 5, 1>> rez {};
  std::ifstream in {filePath};
  auto* line = new char[64];
  do {
    in.getline(line, 64);
    gabe::utils::math::LinearArray<typename K::UnderlyingType, 5, 1> el;
    std::stringstream lineStr {line};
    for (auto idx = 0; idx < 5; ++idx) {
      lineStr >> el[idx][0];
    }
    rez.push_back(el);
  } while (in);

  delete[] line;

  return rez;
}
} // namespace impl

enum class MNISTDataSetType { TEST, TRAIN };

template <concepts::LinearArrayType R> auto loadMNIST(std::string const& filePath, MNISTDataSetType loadFlag)
    -> DataSet<R> {
  FILE* imagesIn = fopen((filePath + "/images").c_str(), "rb");
  FILE* labelsIn = fopen((filePath + "/labels").c_str(), "rb");
  auto expectedImageCount = loadFlag == MNISTDataSetType::TRAIN ? 60000 : 10000;

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
  return DataSet<R> {rezVector};
}

template <concepts::DeepLinearMatrixType I> auto loadCS2Images(std::string const& folderPath, Size imageCount = 2000)
    -> YoloDataSet<I> {
  YoloDataSet<I> images {};
  const std::filesystem::path imgDirectory {folderPath + "/images"};
  const std::filesystem::path labelDirectory {folderPath + "/labels"};
  Size idx = 0;
  for (auto const& dir_entry : std::filesystem::directory_iterator {imgDirectory}) {
    auto fileString = dir_entry.path().filename().string();
    auto lastIdx = fileString.find_last_of(".");
    auto fileName = fileString.substr(0, lastIdx);

    auto data = impl::loadJPEG<I>(dir_entry.path().string());
    auto label = impl::loadCS2Labels<I>(labelDirectory.string() + "/" + fileString.substr(0, lastIdx) + ".txt");
    images.data().emplace_back(data, label);

    if (++idx > imageCount) {
      break;
    }
  }
  return images;
}

template <concepts::LinearArrayType R> auto loadDelimSeparatedFile(std::string const& filePath, char delim = ',')
    -> DataSet<R> {
  DataSet<R> rezVector;

  std::ifstream in {filePath};
  auto* linePtr = new char[1024];
  auto* wordPtr = new char[16];

  while (in.getline(linePtr, 1024)) {
    std::array<typename R::UnderlyingType, R().size()> data;

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

    rezVector.data().emplace_back(R {data}, label);
  }

  delete[] linePtr;
  delete[] wordPtr;

  return rezVector;
}
} // namespace gabe::utils::data
