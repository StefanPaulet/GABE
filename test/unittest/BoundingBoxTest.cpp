//
// Created by stefan on 4/11/24.
//

#include "neural_net/ObjectRecognition.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::nn;
using namespace gabe::utils::math;
} // namespace

TEST(BoundBox, IntersectionOverUnion) {
  BoundingBox a {50, 100, 200, 300};
  BoundingBox b {80, 120, 220, 310};
  auto exp = 0.61714;
  ASSERT_TRUE(Equals<> {}(a.intersectionOverUnion(b), exp));
}
