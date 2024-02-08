//
// Created by stefan on 2/8/24.
//

#include "utils/math/linearArray/LinearArray.hpp"
#include "gtest/gtest.h"

namespace {
using gabe::utils::math::linearArray::array;
}

TEST(LinearArrayTest, Construction) {
  auto arr = array(array(1, 2, 3), array(4, 5, 6));
  (void) arr;
}
