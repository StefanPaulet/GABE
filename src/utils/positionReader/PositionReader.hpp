//
// Created by stefan on 4/18/24.
//

#pragma once
#include <cstdio>
#include <jpeglib.h>
#include <map>
#include <neural_net/NeuralNetwork.hpp>
#include <unistd.h>
#include <utils/data/dataLoader/DataLoader.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

namespace gabe::utils {
class PositionReader {
public:
  static constexpr auto templateCount = 12;
  static constexpr auto imageWidth = 335;
  static constexpr auto imageHeight = 25;
  static constexpr auto templateWidth = 15;
  static constexpr auto offsetX = 53;
  static constexpr auto offsetY = 303;

  using Image = math::LinearArray<int, 3, imageHeight, imageWidth>;
  using TemplateDigit = math::LinearArray<int, 3, imageHeight, templateWidth>;
  using Template = math::LinearArray<int, templateCount, 3, imageHeight, templateWidth>;

  PositionReader() = default;
  explicit PositionReader(Image const& image) : _image {image} {}

  auto trainNetwork(std::string const& folderPath, int imageCount) {
    auto dataSet = gabe::utils::data::loadCoordDigitsImages<TemplateDigit>(folderPath, imageCount);
  }

  auto identifyObjects(std::string const& folderPath, int imageCount) {}

private:
  Image _image {};
};
} // namespace gabe::utils
