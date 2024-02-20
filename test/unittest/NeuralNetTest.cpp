//
// Created by stefan on 2/14/24.
//

#include "neural_net/NeuralNetwork.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
using namespace gabe::nn;
using impl::Dimension;
using impl::NDL;
using linearArray::larray;
} // namespace

TEST(NeuralNetwork, Construction) {
  NeuralNetwork<float, InputLayer<float, Dimension<3>>, Layer<float, IdentityFunction<float>, Dimension<3>>> nn;
  NeuralNetwork<float, NDL<InputLayer, Dimension<3>>, NDL<Layer, IdentityFunction<>, Dimension<3>>> nn1;
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(nn.weights<0>())>, SquareLinearMatrix<float, 3>>);
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(nn.biases<0>())>, LinearColumnArray<float, 3>>);
  static_assert(
      std::is_same_v<std::remove_cvref<decltype(nn.weights<0>())>, std::remove_cvref<decltype(nn1.weights<0>())>>);
  (void) nn;
  (void) nn1;
}

TEST(NeuralNetwork, ForwardPropagating) {
  NeuralNetwork<float, NDL<InputLayer, Dimension<3>>, NDL<Layer, IdentityFunction<>, Dimension<3>>> nn;
  auto input = larray(larray(1.0f, 2, 3));
  nn.weights<0>() = larray(larray(1.0f, 1, 1), larray(1.0f, 1, 1), larray(1.0f, 1, 1));
  nn.biases<0>() = larray(larray(1.0f, 2, 3)).transpose();
  auto rez = nn.feedForward(input.transpose());
  auto expected = larray(larray(7.0f, 8, 9)).transpose();
  ASSERT_EQ(rez, expected);
}
