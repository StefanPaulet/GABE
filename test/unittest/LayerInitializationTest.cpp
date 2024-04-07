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