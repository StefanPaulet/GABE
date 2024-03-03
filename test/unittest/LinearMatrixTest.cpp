//
// Created by stefan on 2/12/24.
//

#include "utils/math/linearArray/LinearArray.hpp"
#include "gtest/gtest.h"

namespace {
using gabe::utils::math::LinearArray;
using gabe::utils::math::LinearColumnArray;
using gabe::utils::math::LinearLineArray;
using gabe::utils::math::LinearMatrix;
using gabe::utils::math::SquareLinearMatrix;
using gabe::utils::math::linearArray::larray;
} // namespace

TEST(LinearMatrixTest, Construction) {
  auto mtrx1 = larray(larray(1, 3, 5), larray(2, 4, 6));
  (void) mtrx1;

  auto mtrx2 = SquareLinearMatrix<int, 2> {{LinearArray<int, 2> {{1, 2}}, LinearArray<int, 2> {{2, 3}}}};
  (void) mtrx2;

  auto lineV = LinearLineArray<int, 3> {{LinearArray<int, 3> {{1, 2, 3}}}};
  auto mtrx3 = larray(larray(1, 2, 3));
  ASSERT_EQ(lineV, mtrx3);

  auto colV =
      LinearColumnArray<int, 3> {{LinearArray<int, 1> {{1}}, LinearArray<int, 1> {{2}}, LinearArray<int, 1> {{3}}}};
  auto mtrx4 = larray(larray(1), larray(2), larray(3));
  ASSERT_EQ(colV, mtrx4);
}

TEST(LinearMatrixTest, Addition) {
  auto mtrx1 = larray(larray(1, 2, 3), larray(4, 5, 6));
  auto mtrx2 = larray(larray(3, 5, 7), larray(2, 4, 6));
  auto mtrx3 = mtrx1 + mtrx2;
  auto mtrx4 = larray(larray(4, 7, 10), larray(6, 9, 12));
  ASSERT_EQ(mtrx3, mtrx4);
}

TEST(LinearMatrixTest, Substraction) {
  auto mtrx1 = larray(larray(10, 15, 20), larray(4, 5, 6));
  auto mtrx2 = larray(larray(9, 8, 7), larray(1, 2, 3));
  auto mtrx3 = mtrx1 - mtrx2;
  auto mtrx4 = larray(larray(1, 7, 13), larray(3, 3, 3));
  ASSERT_EQ(mtrx3, mtrx4);
}

TEST(LinearMatrixTest, Multiplication) {
  auto mtrx1 = larray(larray(1, 2, 3), larray(4, 5, 6));
  auto mtrx2 = larray(larray(2, 2, 2), larray(1, 3, 5));
  auto mtrx3 = mtrx1 * mtrx2;
  auto mtrx4 = larray(larray(2, 4, 6), larray(4, 15, 30));
  ASSERT_EQ(mtrx3, mtrx4);
}

TEST(LinearMatrixTest, Division) {
  auto mtrx1 = larray(larray(10, 3, 2), larray(30, 25, 24));
  auto mtrx2 = larray(larray(3, 1, 2), larray(3, 5, 3));
  auto mtrx3 = mtrx1 / mtrx2;
  auto mtrx4 = larray(larray(3, 3, 1), larray(10, 5, 8));
  ASSERT_EQ(mtrx3, mtrx4);
}

TEST(LinearMatrixTest, Transpose) {
  auto mtrx1 = larray(larray(1, 2, 3), larray(4, 5, 6));
  auto mtrx2 = larray(larray(1, 4), larray(2, 5), larray(3, 6));
  auto mtrx3 = mtrx1.transpose();
  ASSERT_EQ(mtrx2, mtrx3);
}

TEST(LinearMatrixTest, Product) {
  auto mtrx1 = larray(larray(1, 4, 5), larray(2, 3, 6));
  auto mtrx2 = larray(larray(2), larray(2), larray(6));
  auto mtrx3 = larray(larray(40), larray(46));
  auto mtrx4 = mtrx1.product(mtrx2);
  ASSERT_EQ(mtrx3, mtrx4);

  auto mtrx5 = larray(larray(5, 10, 15), larray(2, 3, 4));
  auto mtrx6 = larray(larray(3, 2, 1), larray(4, 5, 8), larray(2, 6, 8));
  auto mtrx7 = larray(larray(85, 150, 205), larray(26, 43, 58));
  auto mtrx8 = mtrx5.product(mtrx6);
  ASSERT_EQ(mtrx7, mtrx8);
}

TEST(LinearMatrixTest, Convolve) {
  auto mtrx1 = larray(larray(1, 1, 1, 0, 0), larray(0, 1, 1, 1, 0), larray(0, 0, 1, 1, 1), larray(0, 0, 1, 1, 0),
                      larray(0, 1, 1, 0, 0));
  auto kern1 = larray(larray(1, 0, 1), larray(0, 1, 0), larray(1, 0, 1));
  auto rez = larray(larray(4, 3, 4), larray(2, 4, 3), larray(2, 3, 4));
  auto mtrx2 = mtrx1.convolve(kern1);
  ASSERT_EQ(mtrx2, rez);
}

TEST(LinearMatrixTest, Unit) {
  auto mtrx1 = SquareLinearMatrix<int, 3>::unit();
  auto mtrx2 = larray(larray(1, 0, 0), larray(0, 1, 0), larray(0, 0, 1));
  ASSERT_EQ(mtrx1, mtrx2);
}

TEST(LinearMatrixTest, Nul) {
  auto mtrx1 = SquareLinearMatrix<int, 3>::nul();
  auto mtrx2 = larray(larray(0, 0, 0), larray(0, 0, 0), larray(0, 0, 0));
  ASSERT_EQ(mtrx1, mtrx2);
}

TEST(LinearMatrixTest, Accumulate) {
  auto accum_sum = [](int x, int y) { return x + y; };
  auto accum_prod = [](int x, int y) { return x * y; };

  auto mtrx1 = larray(larray(1, 2, 4), larray(2, 5, 2));
  ASSERT_EQ(mtrx1.accumulate(0, accum_sum), 16);
  ASSERT_EQ(mtrx1.accumulate(0, accum_prod), 0);
  ASSERT_EQ(mtrx1.accumulate(1, accum_prod), 160);
}