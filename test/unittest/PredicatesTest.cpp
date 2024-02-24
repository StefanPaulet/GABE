//
// Created by stefan on 2/22/24.
//

#include "utils/math/predicates/Predicates.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
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
  auto arrComparator = Equals<std::array<double, 3>> {};
  ASSERT_FALSE(arrComparator(arr1, arr2));
  ASSERT_TRUE(arrComparator(arr1, arr3));
}
