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

TEST(FunctionTest, MeanSquaredError) {
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

TEST(FunctionTest, CrossEntropy) {
  CrossEntropyFunction<> cef;

  static_assert(decltype(cef)::isCostFunction, "MeanSquaredError is not a cost function");

  auto arr1 = larray(larray(0.58, 0.3, 0.12)).transpose();
  auto tarr1 = OneHotEncoder<int, LinearArray<double, 3, 1>> {}(1);
  auto exp1 = -log(0.3);
  ASSERT_EQ(cef(arr1, tarr1).accumulate(0, [](double x, double y) { return x + y; }), exp1);

  auto derivExp1 = -1.0 / 0.3;
  ASSERT_EQ(cef.derive(arr1, tarr1).accumulate(0, [](double x, double y) { return x + y; }), derivExp1);
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

TEST(FunctionTest, SimpleDeepConvolutionFunction) {
  auto mtrx1 = larray(larray(larray(1, 2, 3, 4), larray(5, 6, 7, 8), larray(9, 10, 11, 12), larray(13, 14, 15, 16)));
  auto mtrx2 = larray(larray(larray(2, 1, 3), larray(0, 0, 0), larray(-1, 2, -4)));
  SimpleDeepConvolutionFunction<decltype(mtrx1), decltype(mtrx2)> scf;
  auto rez = larray(larray(-20, -17), larray(-8, -5));
  ASSERT_EQ(scf(mtrx1, mtrx2), rez);

  auto mtrx3 = larray(larray(larray(1, 1, 1), larray(0, 1, 0), larray(1, 1, 1)));
  auto mtrx4 = larray(larray(1, 2, 3), larray(4, 5, 6), larray(7, 8, 9));
  SimpleDeepConvolutionFunction<decltype(mtrx3), LinearArray<int, 1, 1, 1>> scf2;
  auto rez2 = larray(larray(larray(35)));
  ASSERT_EQ(scf2.derive(mtrx3, mtrx4), rez2);

  auto mtrx5 = larray(larray(1, 2, 3), larray(3, 4, 5), larray(5, 6, 7));
  auto mtrx6 = larray(larray(larray(2, 0), larray(0, 1)));
  SimpleDeepConvolutionFunction<LinearArray<int, 1, 4, 4>, decltype(mtrx6)> scf3;
  auto rez3 = larray(larray(larray(1, 2, 3, 0), larray(3, 6, 9, 6), larray(5, 12, 15, 10), larray(0, 10, 12, 14)));
  ASSERT_EQ(scf3.fullyConvolve(mtrx5, mtrx6), rez3);
}

TEST(FunctionTest, FullDeepConvolutionFunction) {
  auto mtrx1 = larray(larray(larray(1, 2, 3), larray(4, 5, 6), larray(7, 8, 9)));
  auto mtrx2 = larray(larray(larray(1, 0, 1), larray(0, 1, 0), larray(1, 0, 1)));
  FullDeepConvolutionFunction<decltype(mtrx1), decltype(mtrx2)> fcf;
  auto rez = larray(larray(6, 12, 8), larray(14, 25, 16), larray(12, 18, 14));
  ASSERT_EQ(fcf(mtrx1, mtrx2), rez);
}

TEST(FunctionTest, StridedDeepConvolutionFunction) {
  auto mtrx1 =
      larray(larray(larray(1, 2, 3, 4, 0), larray(5, 6, 7, 8, 0), larray(9, 10, 11, 12, 0), larray(13, 14, 15, 16, 0)));
  auto mtrx2 = larray(larray(larray(2, 1, 3), larray(0, 0, 0), larray(-1, 2, -4)));
  StridedDeepConvolutionFunction<2, decltype(mtrx1), decltype(mtrx2)> scf;
  auto rez = larray(larray(-20, 23));
  ASSERT_EQ(scf(mtrx1, mtrx2), rez);

  auto mtrx3 = larray(larray(larray(1, 2, 3, 4), larray(5, 6, 7, 8), larray(9, 10, 11, 12), larray(13, 14, 15, 16)));
  auto mtrx4 = larray(larray(2, 1), larray(5, 6));
  StridedDeepConvolutionFunction<2, decltype(mtrx3), LinearArray<int, 1, 2, 2>> scf2;
  auto rez2 = larray(larray(larray(116, 130), larray(172, 186)));
  ASSERT_EQ(scf2.derive(mtrx3, mtrx4), rez2);

  auto mtrx5 = larray(larray(2, 2), larray(3, 1));
  auto mtrx6 = larray(larray(larray(2, 1, 0), larray(5, 6, 0), larray(1, 1, 1)));
  StridedDeepConvolutionFunction<2, LinearArray<int, 1, 5, 5>, decltype(mtrx6)> scf3;
  auto rez3 = larray(larray(larray(2, 2, 4, 2, 2), larray(0, 12, 10, 12, 10), larray(3, 5, 8, 3, 5),
                            larray(0, 18, 15, 6, 5), larray(0, 3, 6, 1, 2)));
  ASSERT_EQ(scf3.fullyConvolve(mtrx5, mtrx6), rez3);
}

TEST(FunctionTest, FullStridedDeepConvolutionFunction) {
  auto mtrx1 = larray(larray(larray(1, 2, 3), larray(4, 5, 6), larray(7, 8, 9)));
  auto mtrx2 = larray(larray(larray(1, 0, 1), larray(0, 1, 0), larray(1, 0, 1)));
  FullStridedDeepConvolutionFunction<2, decltype(mtrx1), decltype(mtrx2)> scf;
  auto rez = larray(larray(6, 8), larray(12, 14));
  ASSERT_EQ(scf(mtrx1, mtrx2), rez);

  auto mtrx3 = larray(larray(larray(1, 2, 3), larray(4, 5, 6), larray(7, 8, 9)));
  auto mtrx4 = larray(larray(2, 1), larray(2, 3));
  FullStridedDeepConvolutionFunction<2, decltype(mtrx3), LinearArray<int, 1, 3, 3>> scf2;
  auto rez2 = larray(larray(larray(15, 26, 10), larray(26, 46, 20), larray(5, 14, 10)));
  ASSERT_EQ(scf2.deriveConvolve(mtrx3, mtrx4), rez2);

  auto mtrx5 = larray(larray(1, -1), larray(2, -2));
  auto mtrx6 = larray(larray(larray(1, 2, 3), larray(4, 5, 6), larray(7, 8, 9)));
  FullStridedDeepConvolutionFunction<2, LinearArray<int, 1, 3, 3>, decltype(mtrx6)> scf3;
  auto rez3 = larray(larray(larray(5, -2, -5), larray(18, -6, -18), larray(10, -4, -10)));
  ASSERT_EQ(scf3.paddedConvolve(mtrx5, mtrx6), rez3);
}

TEST(FunctionTest, Relu) {
  ReluFunction<> rf;
  auto arr1 = larray(1, -5, 3);
  auto arr2 = larray(1, 0, 3);
  ASSERT_EQ(arr1.transform(rf), arr2);

  auto darr1 = larray(-1.3, 2.22, 3.4);
  auto darr2 = larray(0, 2.22, 3.4);
  ASSERT_EQ(darr1.transform(rf), darr2);

  ASSERT_EQ(rf.derive(5), 1);
  ASSERT_EQ(rf.derive(-3), 0);
}

TEST(FunctionTest, MaxPoolFunction) {
  auto mtrx1 = larray(larray(larray(2, 2, 7, 3), larray(9, 4, 6, 1), larray(8, 5, 2, 4), larray(3, 1, 2, 6)));
  auto mtrx2 = larray(larray(larray(9, 7), larray(8, 6)));
  auto mtrx3 = larray(larray(larray(3, 2), larray(-5, 6)));
  MaxPoolFunction<decltype(mtrx1), 2, 2> mpf;
  ASSERT_EQ(mpf(mtrx1), mtrx2);
  auto mtrx4 = larray(larray(larray(0, 0, 2, 0), larray(3, 0, 0, 0), larray(-5, 0, 0, 0), larray(0, 0, 0, 6)));
  ASSERT_EQ(mpf.derive(mtrx1, mtrx2, mtrx3), mtrx4);
}
