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
  auto isUnderlyingType =
      std::is_same_v<typename decltype(larray(larray(1, 2, 3), larray(4, 5, 6)))::UnderlyingType, int>;
  ASSERT_TRUE(isUnderlyingType);
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
  ASSERT_EQ(d_arr3, d_arr4);

  auto eq_arr1 = larray(1, 2, 3);
  auto eq_arr2 = larray(2, 3, 4);
  auto eq_arr3 = larray(3, 5, 7);
  eq_arr2 += eq_arr1;
  ASSERT_EQ(eq_arr2, eq_arr3);

  auto scalar = 1.32;
  auto scalar_arr1 = larray(2.0, 3.12, 4.25);
  auto scalar_arr2 = larray(3.32, 4.44, 5.57);
  auto scalar_arr3 = scalar_arr1 + scalar;
  ASSERT_EQ(scalar_arr2, scalar_arr3);
  scalar_arr3 = scalar + scalar_arr1;
  ASSERT_EQ(scalar_arr2, scalar_arr3);
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
  ASSERT_EQ(d_arr3, d_arr4);

  auto eq_arr1 = larray(5, 5, 5);
  auto eq_arr2 = larray(2, 3, 4);
  auto eq_arr3 = larray(-3, -2, -1);
  eq_arr2 -= eq_arr1;
  ASSERT_EQ(eq_arr2, eq_arr3);

  auto scalar = 2.17;
  auto scalar_arr1 = larray(1.2, 3.33, 2.18);
  auto scalar_arr2 = larray(-0.97, 1.16, 0.01);
  auto scalar_arr3 = larray(0.97, -1.16, -0.01);
  auto scalar_arr4 = scalar_arr1 - scalar;
  ASSERT_EQ(scalar_arr2, scalar_arr4);
  scalar_arr4 = scalar - scalar_arr1;
  ASSERT_EQ(scalar_arr3, scalar_arr4);
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
  ASSERT_EQ(d_arr3, d_arr4);

  auto eq_arr1 = larray(1, 2, 3);
  auto eq_arr2 = larray(3, 2, 3);
  auto eq_arr3 = larray(3, 4, 9);
  eq_arr2 *= eq_arr1;
  ASSERT_EQ(eq_arr2, eq_arr3);

  auto scalar = 0.1;
  auto scalar_arr1 = larray(2.13, 31.12, -0.52);
  auto scalar_arr2 = larray(0.213, 3.112, -0.052);
  auto scalar_arr3 = scalar_arr1 * scalar;
  ASSERT_EQ(scalar_arr2, scalar_arr3);
  scalar_arr3 = scalar * scalar_arr1;
  ASSERT_EQ(scalar_arr2, scalar_arr3);
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
  ASSERT_EQ(d_arr3, d_arr4);

  auto eq_arr1 = larray(3, 5, 4);
  auto eq_arr2 = larray(10, 15, 12);
  auto eq_arr3 = larray(3, 3, 3);
  eq_arr2 /= eq_arr1;
  ASSERT_EQ(eq_arr2, eq_arr3);

  auto scalar = 3;
  auto scalar_arr1 = larray(1.2, 3.33, 2.16);
  auto scalar_arr2 = larray(0.4, 1.11, 0.72);
  auto scalar_arr3 = larray(2.5, 0.9009, 1.3888);
  auto scalar_arr4 = scalar_arr1 / scalar;
  ASSERT_EQ(scalar_arr2, scalar_arr4);
  scalar_arr4 = scalar / scalar_arr1;
  ASSERT_EQ(scalar_arr3, scalar_arr4);
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
  ASSERT_EQ(d_arr3, d_arr4);
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
  ASSERT_EQ(d_arr2, d_arr3);

  auto mdt = MyDoubleTransform();
  auto d_arr4 = larray(larray(1.0, 4, 9), larray(2.1, 4.2, 5.55));
  auto d_arr5 = d_arr4.project(mdt);
  auto d_arr6 = larray(larray(1.0 / 3, 4.0 / 3, 3), larray(0.7, 1.4, 1.85));
  ASSERT_EQ(d_arr5, d_arr6);
}

TEST(LinearArrayTest, Unit) {
  auto arr1 = LinearArray<int, 3>::unit();
  auto arr2 = larray(1, 1, 1);
  ASSERT_EQ(arr1, arr2);

  auto const d_arr1 = LinearArray<double, 3>::unit();
  for (auto const& e : d_arr1) {
    ASSERT_DOUBLE_EQ(e, 1.0);
  }
}

TEST(LinearArrayTest, Nul) {
  auto arr1 = LinearArray<int, 3>::nul();
  auto arr2 = larray(0, 0, 0);
  ASSERT_EQ(arr1, arr2);

  auto const d_arr1 = LinearArray<double, 3>::nul();
  for (auto const& e : d_arr1) {
    ASSERT_DOUBLE_EQ(e, .0);
  }
}

TEST(LinearArrayTest, Accumulate) {
  auto accum_sum = [](int x, int y) { return x + y; };
  auto accum_prod = [](int x, int y) { return x * y; };

  auto arr1 = larray(1, 2, 4);
  ASSERT_EQ(arr1.accumulate(0, accum_sum), 7);
  ASSERT_EQ(arr1.accumulate(0, accum_prod), 0);
  ASSERT_EQ(arr1.accumulate(1, accum_prod), 8);
}

TEST(LinearArrayTest, Max) {
  auto arr1 = larray(1, 2, -3);
  ASSERT_EQ(arr1.max(), 2);

  auto d_arr1 = larray(0.1, 0.32, 0.23);
  ASSERT_EQ(d_arr1.max(), 0.32);
}