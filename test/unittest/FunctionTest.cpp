//
// Created by stefan on 2/21/24.
//

#include "utils/math/function/Function.hpp"
#include "utils/math/function/FunctionTraits.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
} // namespace

TEST(FunctionTest, Sigmoid) {
  SigmoidFunction<> sig;
  SigmoidFunction<float> fSig;

  static_assert(decltype(fSig)::isActivationFunction, "Sigmoid is not an activation function");

  auto exp1 = 0.5f;
  ASSERT_DOUBLE_EQ(sig(.0f), exp1);
  ASSERT_DOUBLE_EQ(fSig(.0f), exp1);

  auto exp2 = 0.25f;
  ASSERT_DOUBLE_EQ(sig.derive(.0f), exp2);
  ASSERT_DOUBLE_EQ(fSig.derive(.0f), exp2);

  SigmoidFunction<int> invalidSig;
  static_assert(impl::is_derivable<decltype(invalidSig)>::value == false);
  static_assert(impl::is_callable<decltype(invalidSig)>::value == false);
  (void) invalidSig;
}

TEST(FunctionTest, Identity) {
  IdentityFunction<> idt;
  IdentityFunction<int> iIdt;

  auto exp1 = 5;
  ASSERT_EQ(iIdt(5), exp1);
  ASSERT_EQ(idt(5), exp1);

  auto exp2 = 1;
  ASSERT_EQ(iIdt.derive(5), exp2);
  ASSERT_EQ(idt.derive(5), exp2);
}