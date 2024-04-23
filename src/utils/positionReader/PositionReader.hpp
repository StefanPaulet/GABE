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
  using Digit = math::LinearArray<float, imageHeight, templateWidth>;

public:
  using Image = math::LinearArray<float, imageHeight, imageWidth>;
  using Template = math::LinearArray<float, templateCount, imageHeight, templateWidth>;

  PositionReader() = default;

  auto loadTemplates(std::string const& folderPath, int imageCount) {
    static constexpr float totalPixels = imageHeight * templateWidth;
    auto dataSet = gabe::utils::data::loadCoordDigitsImages<TemplateDigit>(folderPath, imageCount);
    for (auto const& e : dataSet.data()) {
      Image activatedImage {};
      for (auto lIdx = 0; lIdx < imageHeight; ++lIdx) {
        for (auto cIdx = 0; cIdx < templateWidth; ++cIdx) {
          activatedImage[lIdx][cIdx] = isActive(e.data[0][lIdx][cIdx], e.data[1][lIdx][cIdx], e.data[2][lIdx][cIdx]);
        }
      }
      auto activatedPixels = activatedImage.accumulate(.0f, [](float x, float y) { return x + y; });
      for (auto lIdx = 0; lIdx < imageHeight; ++lIdx) {
        for (auto cIdx = 0; cIdx < templateWidth; ++cIdx) {
          _templates[e.label][lIdx][cIdx] +=
              isActive(e.data[0][lIdx][cIdx], e.data[1][lIdx][cIdx], e.data[2][lIdx][cIdx])
              ? (1.0f / activatedPixels)
              : (-1.0f / (totalPixels - activatedPixels));
        }
      }
    }
    _templates.serialize("templateDigits.gabe");
  }

  auto identifyObjects(std::string const& folderPath, int imageCount) {
    _templates = Template::deserialize("templateDigits.gabe");
    auto dataSet = gabe::utils::data::loadCoordDigitsImages<TemplateDigit>(folderPath, imageCount);
    for (auto const& currTest : dataSet.data()) {

      Digit activatedImage {};
      for (auto lIdx = 0; lIdx < imageHeight; ++lIdx) {
        for (auto cIdx = 0; cIdx < templateWidth; ++cIdx) {
          activatedImage[lIdx][cIdx] =
              currTest.data[0][lIdx][cIdx] + currTest.data[1][lIdx][cIdx] + currTest.data[2][lIdx][cIdx];
        }
      }
      std::cout << currTest.label << '\n';
      for (auto idx = 0; idx < templateCount; ++idx) {
        auto val = activatedImage.convolve(_templates[idx]);
        std::cout << "Template " << idx << " gives values " << val << '\n';
      }
    }
  }

private:
  static auto isActive(float red, float green, float blue) -> bool {
    static constexpr auto limit = 240.0f;
    return red >= limit && green >= limit && blue >= limit;
  }

  static auto isActive(float pixel) -> bool {
    static constexpr auto limit = 690.0f;
    return pixel >= limit;
  }

  Template _templates {};
};
} // namespace gabe::utils
