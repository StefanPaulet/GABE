//
// Created by stefan on 2/14/24.
//

#include "neural_net/NeuralNetwork.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
using namespace gabe::nn;
using gabe::nn::impl::Dimension;
using gabe::nn::impl::NDL;
using linearArray::larray;
} // namespace

TEST(InitializationSchemeTest, HeInitialization) {
  LinearArray<double, 1, 1, 3, 3> kernelArr;
  HeInitialization<> he;
  he(kernelArr, 5);
  auto maxValue = kernelArr.max();
  auto minValue = kernelArr.min();
  auto aproxLimit = 3.0 / sqrt(45);
  ASSERT_TRUE(maxValue < aproxLimit);
  ASSERT_TRUE(std::abs(minValue) < aproxLimit);

  LinearArray<double, 25, 35> weights;
  he(weights);
  auto maxValue1 = weights.max();
  auto minValue1 = weights.min();
  auto aproxLimit1 = 4.0 / sqrt(35);
  ASSERT_TRUE(maxValue1 < aproxLimit1);
  ASSERT_TRUE(std::abs(minValue1) < aproxLimit1);
}

TEST(InitializationSchemeTest, NoInitialization) {
  auto arr = larray(1.0, 2.0, 3.0);
  auto oldArr = arr;
  NoInitialization ni;
  ni(arr);
  ASSERT_EQ(oldArr, arr);
}

TEST(InitializationTest, UniformInitialization) {
  LinearArray<double, 3, 5> arr;
  int lowerBound = -2;
  int upperBound = 2;
  UniformInitialization<-2, 2> ui;
  ui(arr);
  auto maxVal = arr.max();
  auto minVal = arr.min();
  ASSERT_TRUE(static_cast<double>(lowerBound) < minVal);
  ASSERT_TRUE(static_cast<double>(upperBound) > maxVal);
}