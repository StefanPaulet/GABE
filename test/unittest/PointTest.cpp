//
// Created by stefan on 5/6/24.
//


#include "types.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe;
}

TEST(Point, Construction) {
  Point p0 {};
  Point p1 {10, 20};
  Point p2 {p1};
  ASSERT_EQ(p1, p2);
  (void) p0;
}

TEST(Point, Addition) {
  Point p1 {10, 20};
  Point p2 {30, 40};
  Point p3 {p1 + p2};
  Point target {40, 60};
  ASSERT_EQ(p3, target);

  Point p4 {5, -2};
  p4 += p2;
  Point target2 {35, 38};
  ASSERT_EQ(p4, target2);
}

TEST(Point, Division) {
  Point p1 {10, 20};
  Point p2 {p1 / 2};
  Point target {5, 10};
  ASSERT_EQ(p2, target);

  Point p3 {12, 36};
  p3 /= 12;
  Point target2 {1, 3};
  ASSERT_EQ(p3, target2);
}

TEST(Point, Multiplication) {
  Point p1 {10, 20};
  Point p2 {p1 * 4};
  Point target {40, 80};
  ASSERT_EQ(p2, target);

  Point p3 {12, 36};
  p3 *= 5;
  Point target2 {60, 180};
  ASSERT_EQ(p3, target2);
}
