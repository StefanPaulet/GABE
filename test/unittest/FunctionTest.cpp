//
// Created by stefan on 2/21/24.
//

#include "utils/math/function/Function.hpp"
#include "utils/math/function/FunctionTraits.hpp"
#include "utils/math/linearArray/LinearArray.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
using gabe::utils::math::linearArray::larray;
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

TEST(FunctionTest, MeanSquaredErrorTest) {
  MeanSquaredErrorFunction<> msq;

  static_assert(decltype(msq)::isCostFunction, "MeanSquaredError is not a cost function");

  ASSERT_EQ(msq(1, 2), 0);
  ASSERT_EQ(msq(3, -5), 32);

  ASSERT_DOUBLE_EQ(msq(1.0, 2.0), 0.5);
  ASSERT_DOUBLE_EQ(msq(0.58, 0.96), 0.0722);

  auto arr1 = larray(1, 2, 3);
  auto arr2 = larray(0, 2, 0);
  auto exp = larray(0, 0, 4);
  auto derive_exp = larray(1, 0, 3);
  ASSERT_EQ(msq(arr1, arr2), exp);
  ASSERT_EQ(msq.derive(arr1, arr2), derive_exp);

  auto d_arr1 = larray(0.32, 0.4, 0.28);
  auto d_arr2 = larray(1.0, 0, 0);
  auto d_exp = larray(0.2312, 0.08, 0.0392);
  auto d_derive_exp = larray(-0.68, 0.4, 0.28);
  ASSERT_EQ(msq(d_arr1, d_arr2), d_exp);
  ASSERT_EQ(msq.derive(d_arr1, d_arr2), d_derive_exp);

  auto ld_arr1 = larray(larray(0.32, 0.4, 0.28));
  auto ld_arr2 = larray(larray(1.0, 0, 0));
  auto ld_exp = larray(larray(0.2312, 0.08, 0.0392));
  auto ld_derive_exp = larray(larray(-0.68, 0.4, 0.28));
  ASSERT_EQ(msq(ld_arr1, ld_arr2), ld_exp);
  ASSERT_EQ(msq.derive(ld_arr1, ld_arr2), ld_derive_exp);
}

TEST(FunctionTest, Softmax) {
  SoftmaxFunction<> sfm;

  static_assert(decltype(sfm)::isActivationFunction, "Softmax is not an activation function");

  auto arr1 = larray(-1.0, 0, 3, 5);
  auto arr2 = larray(0.0022, 0.0059, 0.1182, 0.8737);
  auto derive_sfm = larray(0.0022, 0.0059, 0.1042, 0.1103);
  ASSERT_EQ(sfm(arr1), arr2);
  ASSERT_EQ(sfm.derive(arr1), derive_sfm);

  auto mtrx1 = larray(larray(6.0f, 2, 1), larray(4.0f, 2, 0));
  auto mtrx2 = larray(larray(0.8466f, 0.0155f, 0.0057f), larray(0.1146f, 0.0155f, 0.0021f));
  ASSERT_EQ(sfm(mtrx1), mtrx2);
}

TEST(FunctionTest, StringToIntegral) {
  std::string integer = "1234";
  ASSERT_EQ(1234, StringToIntegral<int> {}(integer));

  std::string floating = "432.2";
  ASSERT_TRUE(Equals<> {}(432.2, StringToIntegral<double> {}(floating)));

  std::string neg_integer = "-1864";
  ASSERT_EQ(-1864, StringToIntegral<int> {}(neg_integer));

  std::string neg_floating = "-22.34";
  ASSERT_TRUE(Equals<> {}(-22.34, StringToIntegral<double> {}(neg_floating)));
}

TEST(FunctionTest, OneHotEncoder) {
  OneHotEncoder<int, LinearArray<int, 3, 1>> ohe;

  auto arr1 = larray(larray(1, 0, 0)).transpose();
  auto arr2 = ohe(0);
  ASSERT_EQ(arr1, arr2);

  auto sub_one = [](int value) { return value - 1; };
  OneHotEncoder<int, LinearArray<int, 3, 1>, decltype(sub_one)> ohe1;
  auto arr3 = ohe1(1);
  ASSERT_EQ(arr1, arr3);
}

TEST(FunctionTest, SoftMaxDecoder) {
  SoftMaxDecoder<int, LinearArray<double, 3, 1>> smd;
  auto darr1 = larray(larray(0.3, 0.4, 0.3)).transpose();
  auto val = 1;
  ASSERT_EQ(smd(darr1), val);

  auto add_one = [](int value) { return value + 1; };
  SoftMaxDecoder<int, LinearArray<double, 3, 1>, decltype(add_one)> smd1;
  auto val1 = 2;
  ASSERT_EQ(smd1(darr1), val1);
}