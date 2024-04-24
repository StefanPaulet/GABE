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

  PositionReader() { _templates = Template::deserialize("templateDigits.gabe"); }

  auto loadTemplates(std::string const& folderPath) {
    static constexpr auto imageCount = 3;
    static constexpr float totalPixels = imageHeight * templateWidth;
    for (auto dataSet = gabe::utils::data::loadCoordDigitsImages<TemplateDigit>(folderPath, imageCount);
         auto const& e : dataSet.data()) {
      Image activatedImage {};
      for (auto lIdx = 0; lIdx < imageHeight; ++lIdx) {
        for (auto cIdx = 0; cIdx < templateWidth; ++cIdx) {
          activatedImage[lIdx][cIdx] = isActive(e.data[0][lIdx][cIdx], e.data[1][lIdx][cIdx], e.data[2][lIdx][cIdx]);
        }
      }
      auto activatedPixels = activatedImage.accumulate(.0f, [](float x, float y) { return x + y; });
      for (auto lIdx = 0; lIdx < imageHeight; ++lIdx) {
        for (auto cIdx = 0; cIdx < templateWidth; ++cIdx) {
          _templates[e.label][lIdx][cIdx] =
              isActive(e.data[0][lIdx][cIdx], e.data[1][lIdx][cIdx], e.data[2][lIdx][cIdx])
              ? (1.0f / activatedPixels)
              : (-1.0f / (totalPixels - activatedPixels));
        }
      }
    }
    _templates[11].transform([](float x) {
      if (x > .0f) {
        return 1.0f / 9;
      }
      return -1.0f / (totalPixels - 9 - 6 * imageHeight);
    });
    for (auto lIdx = 0; lIdx < imageHeight; ++lIdx) {
      for (auto cIdx = 0; cIdx < 3; ++cIdx) {
        _templates[11][lIdx][cIdx] = _templates[11][lIdx][templateWidth - cIdx - 1] = 0;
      }
    }
    _templates.serialize("templateDigits.gabe");
  }

  auto identifyObjects(Image const& image) const {
    PredictionMaker predictionMaker {_templates};
    Prediction lastPrediction {};
    for (auto leftMargin = 0; leftMargin < imageWidth - templateWidth;) {
      Digit currentDigit = getCurrentDigit(image, leftMargin);
      auto currentPrediction = predictionMaker.getPrediction(currentDigit);

      if (lastPrediction.value > currentPrediction.value && lastPrediction.isValid()) {
        std::cout << "Found an object of label " << lastPrediction.label << " at margin " << leftMargin << '\n';
        predictionMaker.advance(lastPrediction);
        auto marginOffset = predictionMaker.getOffset(lastPrediction);
        leftMargin += marginOffset;
        lastPrediction = {};
      } else {
        ++leftMargin;
        lastPrediction = currentPrediction;
      }
    }
  }

private:
  static auto isActive(float red, float green, float blue) -> bool {
    static constexpr auto limit = 240.9f;
    return red >= limit && green >= limit && blue >= limit;
  }

  static auto getCurrentDigit(Image const& image, int leftMargin) -> Digit {
    Digit rez {};
    for (auto lIdx = 0; lIdx < imageHeight; ++lIdx) {
      for (auto cIdx = 0; cIdx < templateWidth; ++cIdx) {
        rez[lIdx][cIdx] = image[lIdx][cIdx + leftMargin];
      }
    }
    return rez;
  }

  Template _templates {};

  struct Prediction {
    static constexpr std::array<float, 12> threshHold {150.0f, 150.0f, 150.0f, 150.0f, 150.0f, 150.0f,
                                                       150.0f, 150.0f, 150.0f, 150.0f, 150.0f, 190.0f};
    int label {-1};
    float value {-1024.0f};

    [[nodiscard]] auto isValid() const -> bool { return value > threshHold[label]; }
    [[nodiscard]] auto isDigit() const -> bool { return label >= 0 && label <= 9; }
    [[nodiscard]] auto isMinus() const -> bool { return label == 10; }
    [[nodiscard]] auto isDot() const -> bool { return label == 11; }
  };
  struct PredictionMaker {
    explicit PredictionMaker(Template const& templates) : _templates {templates} {}

    enum class State { START, DIGIT, MINUS, DOT, FIRST_AFTER_DOT, SECOND_AFTER_DOT };
    State _state {State::START};
    Template const& _templates;
    [[nodiscard]] auto getPrediction(Digit const& digit) const -> Prediction {
      Prediction currentPrediction {-1, -1024.0f};
      std::vector<int> unavailableTemplates {};
      switch (_state) {
        using enum State;
        case DIGIT: {
          unavailableTemplates.push_back(10);
          break;
        }
        case DOT:
        case FIRST_AFTER_DOT:
        case MINUS: {
          unavailableTemplates.push_back(10);
          [[fallthrough]];
        }
        case START:
        case SECOND_AFTER_DOT: {
          unavailableTemplates.push_back(11);
          break;
        }
        default: {
          break;
        }
      }
      for (auto idx = 0; idx < templateCount; ++idx) {
        if (std::ranges::find(unavailableTemplates, idx) == unavailableTemplates.end()) {
          auto currentValue = digit.convolve(_templates[idx])[0][0];
          if (currentValue > currentPrediction.value) {
            currentPrediction = {idx, currentValue};
          }
        }
      }
      return currentPrediction;
    }

    [[nodiscard]] auto getOffset(Prediction const& prediction) const -> int {
      auto offset = 12;
      if (_state == State::SECOND_AFTER_DOT) {
        offset = 20;
      } else {
        if (prediction.isMinus() || prediction.isDot()) {
          offset = 8;
        }
      }
      if (prediction.label == 1 || prediction.label == 7) {
        offset -= 4;
      }
      if (prediction.label == 4) {
        offset -= 2;
      }
      return offset;
    }

    auto advance(Prediction const& prediction) -> void {
      switch (_state) {
        using enum State;
        case START:
        case DIGIT: {
          if (prediction.isDigit()) {
            _state = DIGIT;
          }
          if (prediction.isMinus()) {
            _state = MINUS;
          }
          if (prediction.isDot()) {
            _state = DOT;
          }
          break;
        }
        case DOT: {
          if (prediction.isDigit()) {
            _state = FIRST_AFTER_DOT;
          }
          break;
        }
        case FIRST_AFTER_DOT: {
          if (prediction.isDigit()) {
            _state = SECOND_AFTER_DOT;
          }
          break;
        }
        case MINUS:
        case SECOND_AFTER_DOT: {
          if (prediction.isDigit()) {
            _state = DIGIT;
          }
          if (prediction.isMinus()) {
            _state = MINUS;
          }
          break;
        }
        default: {
          break;
        }
      }
    }
  };
};
} // namespace gabe::utils
