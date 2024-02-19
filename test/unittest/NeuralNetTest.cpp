//
// Created by stefan on 2/14/24.
//

#include "neural_net/NeuralNetwork.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
using namespace gabe::nn;
using namespace layer;
using linearArray::larray;
} // namespace

TEST(NeuralNetwork, Construction) {
  NeuralNetwork<float, InputLayer<float, 3>, Layer<float, IdentityFunction<float>, 3>> nn;
  static_assert(std::is_same_v<decltype(nn.weights(0)), SquareLinearMatrix<float, 3>&>);
  static_assert(std::is_same_v<decltype(nn.biases(0)), LinearColumnArray<float, 3>&>);
  (void) nn;
}

TEST(NeuralNetwork, ForwardPropagating) {
  NeuralNetwork<float, InputLayer<float, 3>, Layer<float, IdentityFunction<float>, 3>> nn;
  auto input = larray(larray(1.0f, 2, 3));
  nn.weights(0) = larray(larray(1.0f, 1, 1), larray(1.0f, 1, 1), larray(1.0f, 1, 1));
  nn.biases(0) = larray(larray(1.0f, 2, 3)).transpose();
  auto rez = nn.feedForward(input.transpose());
  auto expected = larray(larray(7.0f, 8, 9)).transpose();
  ASSERT_EQ(rez, expected);
}
