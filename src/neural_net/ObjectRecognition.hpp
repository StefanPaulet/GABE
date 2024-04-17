//
// Created by stefan on 4/8/24.
//

#pragma once
#include "NeuralNetwork.hpp"

namespace gabe::nn {

struct BoundingBox {
  BoundingBox() = default;
  BoundingBox(BoundingBox const&) = default;
  BoundingBox(double x1, double y1, double x2, double y2) : _x1 {x1}, _y1 {y1}, _x2 {x2}, _y2 {y2} {}
  explicit BoundingBox(std::array<double, 4> const& arr) {
    auto centerX = arr[0];
    auto centerY = arr[1];
    auto width = arr[2];
    auto height = arr[3];
    _x1 = centerX - width / 2;
    _y1 = centerY - height / 2;
    _x2 = centerX + width / 2;
    _y2 = centerY + height / 2;
  }

  [[nodiscard]] auto area() const { return (_x2 - _x1) * (_y2 - _y1); }

  auto intersectionOverUnion(BoundingBox const& other) const {
    if (empty() || other.empty()) {
      return .0;
    }

    BoundingBox intersection {std::max(_x1, other._x1), std::max(_y1, other._y1), std::min(_x2, other._x2),
                              std::min(_y2, other._y2)};
    if (intersection._x1 >= intersection._x2 || intersection._y1 >= intersection._y2) {
      return .0;
    }

    auto unionSize = area() + other.area() - intersection.area();
    return intersection.area() / unionSize;
  }

  auto empty() const -> bool { return _x1 == 0 && _y1 == 0 && _x2 == 0 && _y2 == 0; }

  auto width() const { return _x2 - _x1; }
  auto height() const { return _y2 - _y1; }

  double _x1 {};
  double _y1 {};
  double _x2 {};
  double _y2 {};
};

class ObjectDetection {
  static constexpr Size gridSize = 7;
  static constexpr Size boundingBoxes = 2;
  static constexpr Size outputSize = gridSize * gridSize * 5 * boundingBoxes;
  using LeakyReluFunction = gabe::utils::math::LeakyReluFunction<>;
  using SigmoidFunction = gabe::utils::math::SigmoidFunction<>;

  struct YoloCostFunction {
    static constexpr auto isCostFunction = true;

  private:
    using Input = gabe::utils::math::LinearArray<double, outputSize, 1>;
    using Target = std::vector<gabe::utils::math::LinearArray<double, 5, 1>>;
    static constexpr auto confidenceThreshold = 0.5;
    static constexpr auto coordFactor = 5.0;
    static constexpr auto presenceFactor = 0.5;

    auto inCell(gabe::utils::math::LinearArray<double, 5, 1> const& target, Size gridIdx) {
      auto gridLine = static_cast<double>(gridIdx / gridSize);
      auto gridColumn = static_cast<double>(gridIdx % gridSize);
      return target[2][0] >= gridLine / gridSize && target[2][0] <= (gridLine + 1) / gridSize
          && target[1][0] >= gridColumn / gridSize && target[1][0] <= (gridColumn + 1) / gridSize;
    }

    auto objectPresent(Target const& target, Size gridIdx) {
      for (auto const& t : target) {
        if (inCell(t, gridIdx)) {
          return BoundingBox {std::array<double, 4> {t[1][0], t[2][0], t[3][0], t[4][0]}};
        }
      }
      return BoundingBox {0, 0, 0, 0};
    }

  public:
    auto operator()(Input const& in, Target const& target) {
      //empty on purpose
    }

    auto derive(Input const& in, Target const& target) {
      constexpr auto coordCoef = 5.0;
      constexpr auto noObjCoef = 0.5;
      Input gradient {};
      for (auto gridIdx = 0; gridIdx < gridSize * gridSize; ++gridIdx) {
        auto currGrid = gridIdx * 5 * boundingBoxes;
        auto gridObjectBox = objectPresent(target, gridIdx);
        if (!gridObjectBox.empty()) {
          auto responsibleBoxIdx = 0;
          auto responsibleBoxConfidence = .0;
          auto responsibleBox = BoundingBox {std::array<double, 4> {in[currGrid + 1][0], in[currGrid + 2][0],
                                                                    in[currGrid + 3][0], in[currGrid + 4][0]}};
          for (auto boxIdx = 0; boxIdx < boundingBoxes; ++boxIdx) {
            BoundingBox currBox {
                std::array<double, 4> {in[currGrid + boxIdx * 5 + 1][0], in[currGrid + boxIdx * 5 + 2][0],
                                       in[currGrid + boxIdx * 5 + 3][0], in[currGrid + boxIdx * 5 + 4][0]}};
            auto currBoxConfidence = in[currGrid + boxIdx * 5][0] * gridObjectBox.intersectionOverUnion(currBox);
            if (currBoxConfidence > responsibleBoxConfidence) {
              responsibleBoxConfidence = currBoxConfidence;
              responsibleBoxIdx = boxIdx;
              responsibleBox = currBox;
            }
          }
          for (auto boxIdx = 0; boxIdx < boundingBoxes; ++boxIdx) {
            if (boxIdx == responsibleBoxIdx) {
              if (responsibleBox.width() == 0 || responsibleBox.height() == 0) {
                std::cout << "Fuckup incoming\n";
                std::terminate();
              }
              gradient[currGrid + boxIdx * 5][0] = 2 * (responsibleBoxConfidence - 1);
              gradient[currGrid + boxIdx * 5 + 1][0] = coordCoef * 2 * (responsibleBox._x1 - gridObjectBox._x1);
              gradient[currGrid + boxIdx * 5 + 2][0] = coordCoef * 2 * (responsibleBox._y1 - gridObjectBox._y1);
              gradient[currGrid + boxIdx * 5 + 3][0] =
                  (sqrt(responsibleBox.width()) - gridObjectBox.width()) / sqrt(responsibleBox.width());
              gradient[currGrid + boxIdx * 5 + 4][0] =
                  (sqrt(responsibleBox.height()) - gridObjectBox.height()) / sqrt(responsibleBox.height());
            }
          }
        } else {
          for (auto boxIdx = 0; boxIdx < boundingBoxes; ++boxIdx) {
            gradient[currGrid + boxIdx * 5][0] = noObjCoef * 2 * in[currGrid + boxIdx * 5][0];
          }
        }
      }
      return gradient;
    }
  };

public:
  using InitializedObjectRecongnitionNet =
      NeuralNetwork<double, ConvolutionalInputLayer<640, 3>,
                    FullStridedConvolutionalLayer<32, 7, 2, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    FullStridedConvolutionalLayer<32, 5, 2, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<64, 3, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<64, 3, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<128, 3, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<256, 3, LeakyReluFunction, HeInitialization<>>,
                    InitSizedLayer<1024, Layer, HeInitialization<>, LeakyReluFunction>,
                    InitSizedLayer<outputSize, OutputLayer, XavierInitialization<>, SigmoidFunction, YoloCostFunction>>;
  using ObjectRecongnitionNet =
      NeuralNetwork<double, ConvolutionalInputLayer<640, 3>, FullStridedConvolutionalLayer<32, 7, 2, LeakyReluFunction>,
                    MaxPoolLayer<2, 2>, FullStridedConvolutionalLayer<32, 5, 2, LeakyReluFunction>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<64, 3, LeakyReluFunction>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<64, 3, LeakyReluFunction>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<128, 3, LeakyReluFunction>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<256, 3, LeakyReluFunction>, SizedLayer<1024, Layer, LeakyReluFunction>,
                    SizedLayer<outputSize, OutputLayer, SigmoidFunction, YoloCostFunction>>;
};
} // namespace gabe::nn
