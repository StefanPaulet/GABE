//
// Created by stefan on 4/11/24.
//

#include "utils/data/dataLoader/DataLoader.hpp"
#include "utils/math/linearArray/LinearArray.hpp"
#include "utils/objectDetection/ObjectDetectionController.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils;
using namespace gabe::utils::math;
} // namespace

TEST(ObjectDetection, Detection) {
  auto img = data::impl::loadJPEG<LinearArray<unsigned char, 3, 640, 640>>("../../../data/testImages/example.jpg");
  auto* data = new unsigned char[img.total_size()];
  for (auto channel = 0; channel < 3; ++channel) {
    for (auto line = 0; line < 640; ++line) {
      for (auto col = 0; col < 640; ++col) {
        data[line * 640 * 3 + col * 3 + channel] = img[channel][line][col];
      }
    }
  }

  ObjectDetectionController objectDetectionController {"../../../scripts/objectDetection"};
  auto result = objectDetectionController.analyzeImage(data);
  auto expected = BoundingBox {{200, 268}, {300, 506}};

  ASSERT_EQ(result.size(), 1);
  ASSERT_EQ(result[0], expected);

  delete[] data;
}
