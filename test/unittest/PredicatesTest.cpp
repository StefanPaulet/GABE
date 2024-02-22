//
// Created by stefan on 2/22/24.
//

#include "utils/math/predicates/Predicates.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
}

TEST(PredicatesTest, Equal) {
  ASSERT_FALSE(equals<int>(1, 2));
  ASSERT_TRUE(equals<int>(1, 1));
  ASSERT_TRUE(equals<double>(5.43, 5.43));
  ASSERT_TRUE(equals<double>(5.4300002, 5.43000001));

  struct EqualComparable {
    int x;
    bool operator==(EqualComparable const& o) const = default;
  };
  EqualComparable a {5};
  EqualComparable b {6};
  EqualComparable c {5};
  ASSERT_TRUE(equals<EqualComparable>(a, c));
  ASSERT_FALSE(equals<EqualComparable>(a, b));

  struct NonEqualComparable {
    int x;
    bool operator==(NonEqualComparable const&) = delete;
  };
  NonEqualComparable na {5};
  NonEqualComparable nb {6};
  NonEqualComparable nc {5};
  NonEqualComparable const& nar = na;
  ASSERT_FALSE(equals<NonEqualComparable>(na, nc));
  ASSERT_FALSE(equals<NonEqualComparable>(na, nb));
  ASSERT_TRUE(equals<NonEqualComparable>(na, nar));
}
