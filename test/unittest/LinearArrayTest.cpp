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
  auto arr1 = LinearArray<int, 3> {{1, 2, 3}};
  auto arr2 = larray(4, 5, 6);
  auto arr3 = arr1 + arr2;
  auto arr4 = larray(5, 7, 9);
  ASSERT_EQ(arr3, arr4);

  auto arr5 = arr1 + larray(0, 0, 0);
  ASSERT_EQ(arr5, arr1);

  auto d_arr1 = LinearArray<double, 3> {{2.3, 4.52, 3.2}};
  auto d_arr2 = larray(3.2, 0.38, 1);
  auto d_arr3 = d_arr1 + d_arr2;
  auto d_arr4 = larray(5.5, 4.9, 4.2);

  for (int index = 0; index < d_arr3.data().size(); ++index) {
    ASSERT_DOUBLE_EQ(d_arr3[index], d_arr4[index]);
  }
}

TEST(LinnearArrayTest, Substraction) {
  auto arr1 = LinearArray<int, 3> {{2, 4, 6}};
  auto arr2 = larray(1, 1, 1);
  auto arr3 = arr1 - arr2;
  auto arr4 = larray(1, 3, 5);
  ASSERT_EQ(arr3, arr4);

  auto arr5 = arr1 - larray(0, 0, 0);
  ASSERT_EQ(arr5, arr1);

  auto d_arr1 = LinearArray<double, 3> {{1.89, 3.11, 5.23}};
  auto d_arr2 = larray(1.1, 0.28, 11.53);
  auto d_arr3 = d_arr1 - d_arr2;
  auto d_arr4 = larray(0.79, 2.83, -6.3);
  for (int index = 0; index < d_arr3.data().size(); ++index) {
    ASSERT_DOUBLE_EQ(d_arr3[index], d_arr4[index]);
  }
}

TEST(LinearArrayTest, Multiplication) {
  auto arr1 = LinearArray<int, 3> {{5, 12, 3}};
  auto arr2 = larray(2, 1, 3);
  auto arr3 = arr1 * arr2;
  auto arr4 = larray(10, 12, 9);
  ASSERT_EQ(arr3, arr4);

  auto arr5 = arr1 * larray(1, 1, 1);
  ASSERT_EQ(arr5, arr1);

  auto d_arr1 = LinearArray<double, 3> {{2.3, 3, 5.25}};
  auto d_arr2 = larray(0.1, -1.43, 0.25);
  auto d_arr3 = d_arr1 * d_arr2;
  auto d_arr4 = larray(0.23, -4.29, 1.3125);
  for (int index = 0; index < d_arr3.data().size(); ++index) {
    ASSERT_DOUBLE_EQ(d_arr3[index], d_arr4[index]);
  }
}

TEST(LinnearArrayTest, Division) {
  auto arr1 = LinearArray<int, 3> {{3, 21, 33}};
  auto arr2 = larray(3, 3, 3);
  auto arr3 = arr1 / arr2;
  auto arr4 = larray(1, 7, 11);
  ASSERT_EQ(arr3, arr4);

  auto arr5 = arr1 / larray(1, 1, 1);
  ASSERT_EQ(arr5, arr1);

  auto d_arr1 = LinearArray<double, 3> {{3.4, 5.6, 7.62}};
  auto d_arr2 = larray(2, 1.4, 2);
  auto d_arr3 = d_arr1 / d_arr2;
  auto d_arr4 = larray(1.7, 4, 3.81);
  for (int index = 0; index < d_arr3.data().size(); ++index) {
    ASSERT_DOUBLE_EQ(d_arr3[index], d_arr4[index]);
  }
}

TEST(LinearArrayTest, DotProduct) {
  auto arr1 = LinearArray<int, 3> {{1, 5, 10}};
  auto arr2 = LinearArray<int, 3> {{4, 2, 12}};
  auto val = arr1.dot(arr2);
  ASSERT_EQ(val, 134);

  auto d_arr1 = LinearArray<double, 3> {{3.3, 2.42, 5}};
  auto d_arr2 = LinearArray<double, 3> {{2.2, 1, 5.23}};
  auto d_val = d_arr1.dot(d_arr2);
  ASSERT_DOUBLE_EQ(d_val, 35.83);
}

TEST(LinearArrayTest, Access) {
  auto const arr1 = larray(1, 2, 3);
  ASSERT_EQ(arr1[0], 1);

  auto arr2 = larray(3, 5, 7);
  ASSERT_EQ(arr2[1], 5);

  auto const arr3 = larray(larray(2.0, 3, 4), larray(5, 6, 7.35));
  ASSERT_DOUBLE_EQ(arr3[1][2], 7.35);

  auto arr4 = larray(larray(1, 5, 9.0), larray(1.32, 2, 3));
  ASSERT_DOUBLE_EQ(arr4[0][2], 9.0);
}

TEST(LinearArrayTest, Print) {
  std::stringstream oss;
  auto arr1 = larray(1, 2, 3);
  oss << arr1;
  ASSERT_EQ(oss.str(), "[1, 2, 3]");

  std::stringstream().swap(oss);
  auto arr2 = larray(larray(1, 2, 3), larray(4, 5, 6));
  oss << arr2;
  ASSERT_EQ(oss.str(), "[[1, 2, 3], [4, 5, 6]]");

  std::stringstream().swap(oss);
  auto arr3 = larray(larray(larray(1, 2, 3), larray(4, 5, 6)));
  oss << arr3;
  ASSERT_EQ(oss.str(), "[[[1, 2, 3], [4, 5, 6]]]");
}

TEST(LinearArrayTest, Transform) {
  struct MyIntTransform {
    auto operator()(int x) const -> int { return x * 2; }
  };
  auto arr1 = LinearArray<int, 3> {{2, 3, 4}};
  auto arr2 = larray(4, 6, 8);

  MyIntTransform mit;
  arr1.transform(mit);
  ASSERT_EQ(arr1, arr2);

  auto arr3 = larray(larray(1, 2, 3), larray(4, 6, 8));
  auto arr4 = larray(larray(2, 4, 6), larray(8, 12, 16));
  arr3.transform(mit);
  ASSERT_EQ(arr3, arr4);

  struct MyDoubleTransform {
    auto operator()(double x) const -> double { return x / 3.0; }
  };
  auto d_arr1 = LinearArray<double, 3> {{1.41, 5.31, 9}};
  auto d_arr2 = larray(0.47, 1.77, 3);

  MyDoubleTransform mdt;
  d_arr1.transform(mdt);
  for (int index = 0; index < d_arr2.data().size(); ++index) {
    ASSERT_DOUBLE_EQ(d_arr1[index], d_arr2[index]);
  }

  auto d_arr3 = larray(larray(1.0, 4, 9), larray(2.1, 4.2, 5.55));
  auto d_arr4 = larray(larray(1.0 / 3, 4.0 / 3, 3), larray(0.7, 1.4, 1.85));
  d_arr3.transform(mdt);
  for (int index = 0; index < d_arr3.data().size(); ++index) {
    for (int p_index = 0; p_index < d_arr3.data()[0].data().size(); ++p_index) {
      ASSERT_DOUBLE_EQ(d_arr3[index][p_index], d_arr4[index][p_index]);
    }
  }
}

TEST(LinearArrayTest, Project) {
  struct MyIntTransform {
    auto operator()(int x) const -> int { return x * 2; }
  };
  auto arr1 = LinearArray<int, 3> {{2, 3, 4}};
  auto arr2 = arr1.project(MyIntTransform());
  auto arr3 = larray(4, 6, 8);
  ASSERT_EQ(arr2, arr3);

  auto mit = MyIntTransform();
  auto arr4 = larray(larray(1, 2, 3), larray(4, 6, 8));
  auto arr5 = arr4.project(mit);
  auto arr6 = larray(larray(2, 4, 6), larray(8, 12, 16));
  ASSERT_EQ(arr5, arr6);

  struct MyDoubleTransform {
    auto operator()(double x) const -> double { return x / 3.0; }
  };
  auto d_arr1 = LinearArray<double, 3> {{1.41, 5.31, 9}};
  auto d_arr2 = d_arr1.project(MyDoubleTransform());
  auto d_arr3 = larray(0.47, 1.77, 3);
  for (int index = 0; index < d_arr3.data().size(); ++index) {
    ASSERT_DOUBLE_EQ(d_arr2[index], d_arr3[index]);
  }

  auto mdt = MyDoubleTransform();
  auto d_arr4 = larray(larray(1.0, 4, 9), larray(2.1, 4.2, 5.55));
  auto d_arr5 = d_arr4.project(mdt);
  auto d_arr6 = larray(larray(1.0 / 3, 4.0 / 3, 3), larray(0.7, 1.4, 1.85));
  for (int index = 0; index < d_arr5.data().size(); ++index) {
    for (int p_index = 0; p_index < d_arr5.data()[0].data().size(); ++p_index) {
      ASSERT_DOUBLE_EQ(d_arr5[index][p_index], d_arr6[index][p_index]);
    }
  }
}