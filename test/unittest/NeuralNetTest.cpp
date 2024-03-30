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

TEST(NeuralNetwork, Construction) {
  NeuralNetwork<float, InputLayer<float, Dimension<3>>, Layer<float, IdentityFunction<float>, Dimension<3>>> nn;
  NeuralNetwork<float, NDL<InputLayer, Dimension<3>>, NDL<Layer, IdentityFunction<>, Dimension<3>>> nn1;
  NeuralNetwork<float, SizedLayer<3, InputLayer>, SizedLayer<3, Layer, IdentityFunction<>>> nn2;
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(nn.weights<0>())>, SquareLinearMatrix<float, 3>>);
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(nn.biases<0>())>, LinearColumnArray<float, 3>>);
  static_assert(
      std::is_same_v<std::remove_cvref<decltype(nn.weights<0>())>, std::remove_cvref<decltype(nn1.weights<0>())>>);
  static_assert(
      std::is_same_v<std::remove_cvref<decltype(nn.weights<0>())>, std::remove_cvref<decltype(nn2.weights<0>())>>);
  (void) nn;
  (void) nn1;
  (void) nn2;
}

TEST(NeuralNetwork, ForwardPropagation) {
  NeuralNetwork<float, SizedLayer<3, InputLayer>, SizedLayer<3, Layer, IdentityFunction<>>> nn;
  auto input = larray(larray(1.0f, 2, 3));
  nn.weights<0>() = larray(larray(1.0f, 1, 1), larray(1.0f, 1, 1), larray(1.0f, 1, 1));
  nn.biases<0>() = larray(larray(1.0f, 2, 3)).transpose();
  auto rez = nn.feedForward(input.transpose());
  auto expected = larray(larray(7.0f, 8, 9)).transpose();
  ASSERT_EQ(rez, expected);

  NeuralNetwork<float, SizedLayer<5, InputLayer>, SizedLayer<3, Layer, IdentityFunction<>>,
                SizedLayer<1, Layer, IdentityFunction<>>>
      nn1;
  auto input1 = larray(larray(1.0f, 1, 1, 1, 1));
  weights<0>(nn1) = larray(larray(1.0f, 1, 1, 1, 1), larray(1.0f, 1, 1, 1, 1), larray(1.0f, 1, 1, 1, 1));
  biases<0>(nn1) = larray(larray(1.0f, 2, 3)).transpose();
  weights<1>(nn1) = larray(larray(1.0f), larray(1.0f), larray(1.0f)).transpose();
  auto rez1 = nn1.feedForward(input1.transpose());
  auto expected1 = larray(larray(21.0f));
  ASSERT_EQ(rez1, expected1);

  NeuralNetwork<float, SizedLayer<3, InputLayer>, SizedLayer<3, Layer, SigmoidFunction<>>> nn2;
  auto input2 = larray(larray(1.0f, 1, 1));
  auto rez2 = nn2.feedForward(input2.transpose());
  auto expected2 = larray(larray(0.5f, 0.5f, 0.5f)).transpose();
  ASSERT_EQ(rez2, expected2);
}

TEST(NeuralNetwork, BackPropagation) {
  NeuralNetwork<float, SizedLayer<3, InputLayer>,
                SizedLayer<3, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  auto input = larray(larray(1.0f, 1, 1)).transpose();
  auto target = larray(larray(2.0f, 0, 0)).transpose();
  auto learningRate = 0.05f;
  nn.weights<0>() = larray(larray(1.0f, 1, 1), larray(1.0f, 1, 1), larray(1.0f, 1, 1));
  nn.backPropagate(input, target, learningRate);


  ASSERT_EQ(nn.weights<0>(), larray(larray(.95f, .95f, .95f), larray(.85f, .85f, .85f), larray(.85f, .85f, .85f)));
  ASSERT_EQ(nn.biases<0>(), larray(larray(-.05f, -.15f, -.15f)).transpose());


  NeuralNetwork<float, SizedLayer<2, InputLayer>, SizedLayer<2, Layer, IdentityFunction<>>,
                SizedLayer<1, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn1;
  auto input1 = larray(larray(2.f, 3)).transpose();
  auto target1 = larray(larray(1.f)).transpose();
  nn1.weights<0>() = larray(larray(.11f, .21f), larray(.12f, .08f));
  nn1.weights<1>() = larray(larray(.14f, .15f));

  auto weight11Target = larray(larray(.12132f, .22698f), larray(.13213f, .09820f));
  auto weight12Target = larray(larray(.174383f, .169416f));

  nn1.backPropagate(input1, target1, 0.05f);
  ASSERT_EQ(nn1.weights<0>(), weight11Target);
  ASSERT_EQ(nn1.weights<1>(), weight12Target);
}
