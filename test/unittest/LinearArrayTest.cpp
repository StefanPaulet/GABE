//
// Created by stefan on 2/8/24.
//

#include "utils/math/linearArray/LinearArray.hpp"
#include "gtest/gtest.h"

namespace {
using gabe::utils::math::LinearArray;
using gabe::utils::math::linearArray::larray;
}

TEST(LinearArrayTest, Construction) {
  auto arr1 = larray(larray<int, float, int>(1, 2, 3), larray<float>(4, 5, 6));
  auto isCorrectType = std::is_same_v<decltype(arr1), LinearArray<float, 2, 3>>;
  ASSERT_TRUE(isCorrectType);

  (void) arr1;

  auto arr2 = LinearArray<int, 3>();
  (void) arr2;

  auto arr3 = LinearArray<int, 3> {{1, 2, 3}};
  (void) arr3;

  auto arr4 = LinearArray<int, 2, 3> {{arr2, arr3}};
  (void) arr4;

  auto arr5 = LinearArray<int, 3> {arr2};
  (void) arr5;

  auto arr6 = LinearArray<int, 3> {std::move(arr5)};
  (void) arr6;
}

TEST(LinnearArrayTest, Addition) {
  auto arr1 = larray(1, 2, 3);
  auto arr2 = larray(4, 5, 6);
  auto arr3 = arr1 + arr2;
  auto arr4 = larray(5, 7, 9);

  ASSERT_EQ(arr3, arr4);
}