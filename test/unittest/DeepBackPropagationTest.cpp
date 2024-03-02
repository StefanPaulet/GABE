//
// Created by stefan on 2/29/24.
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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(NeuralNetwork, BackPropagation) {
  NeuralNetwork<float, SizedLayer<3, InputLayer>,
                SizedLayer<3, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  auto input = larray(larray(1.0f, 1, 1)).transpose();
  auto target = larray(larray(2.0f, 0, 0)).transpose();
  auto learningRate = 0.05f;
  nn.weights<0>() = larray(larray(1.0f, 1, 1), larray(1.0f, 1, 1), larray(1.0f, 1, 1));

  auto epochCount = 60;
  for (auto epoch = 0; epoch < epochCount; ++epoch) {
    nn.backPropagate(input, target, learningRate);
  }

  auto output = nn.feedForward(input);
  ASSERT_EQ(target, output);
}
