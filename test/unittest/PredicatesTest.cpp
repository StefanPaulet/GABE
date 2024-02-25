//
// Created by stefan on 2/22/24.
//

#include "utils/math/predicates/Predicates.hpp"
#include "utils/math/linearArray/LinearArray.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
using linearArray::larray;
}

TEST(PredicatesTest, Equal) {
  ASSERT_FALSE(Equals<>()(1, 2));
  ASSERT_TRUE(Equals<>()(1, 1));
  ASSERT_TRUE(Equals<>()(5.43, 5.43));
  ASSERT_TRUE(Equals<>()(5.4300002, 5.43000001));

  struct EqualComparable {
    int x;
    bool operator==(EqualComparable const& o) const = default;
  };
  EqualComparable a {5};
  EqualComparable b {6};
  EqualComparable c {5};
  ASSERT_TRUE(Equals<>()(a, c));
  ASSERT_FALSE(Equals<>()(a, b));

  struct NonEqualComparable {
    int x;
    bool operator==(NonEqualComparable const&) = delete;
  };
  NonEqualComparable na {5};
  NonEqualComparable nb {6};
  NonEqualComparable nc {5};
  NonEqualComparable const& nar = na;
  ASSERT_FALSE(Equals<>()(na, nc));
  ASSERT_FALSE(Equals<>()(na, nb));
  ASSERT_TRUE(Equals<>()(na, nar));

  std::array<double, 3> arr1 {2, 3, 4};
  std::array<double, 3> arr2 {1, 2, 3};
  std::array<double, 3> arr3 {2.000001, 3, 4};
  ASSERT_FALSE(Equals<>()(arr1, arr2));
  ASSERT_TRUE(Equals<>()(arr1, arr3));

  auto mtrx1 = larray(larray(1.0f, 2.000002, 3));
  auto mtrx2 = larray(larray(1.00000001f, 2, 3.0000005));
  auto mtrx3 = larray(larray(1.001f, 2, 3));
  Equals<>()(mtrx1, mtrx2);
  ASSERT_TRUE(Equals<>()(mtrx1, mtrx2));
  ASSERT_FALSE(Equals<>()(mtrx1, mtrx3));
}
