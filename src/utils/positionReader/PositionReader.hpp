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
private:
  using TemplateDigit = math::LinearArray<float, 3, imageHeight, templateWidth>;
public:

  using Image = math::LinearArray<float, imageHeight, imageWidth>;
  using Template = math::LinearArray<float, templateCount, imageHeight, templateWidth>;

  PositionReader() = default;

  auto loadTemplates(std::string const& folderPath, int imageCount) {
    auto dataSet = gabe::utils::data::loadCoordDigitsImages<TemplateDigit>(folderPath, imageCount);
    auto nulVal = TemplateDigit::nul();
    for (auto const& e : dataSet.data()) {
      for (auto lIdx = 0; lIdx < imageHeight; ++lIdx) {
        for (auto cIdx = 0; cIdx < templateWidth; ++cIdx) {
          _templates[e.label]
        }
      }
    }
  }

  auto identifyObjects(std::string const& folderPath, int imageCount) {}

private:
  auto isActive(float red, float green, float blue) {
    static constexpr auto limit = 235.0;
    return red >= limit && green >= limit && blue >= limit;
  }

  Template _templates {};
};
} // namespace gabe::utils
