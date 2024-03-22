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

TEST(ConvolutionalNeuratNetwork, Construction) {
  NeuralNetwork<int, ConvolutionalInputLayer<128, 3>, ConvolutionalLayer<8, 5, 1>,
                SizedLayer<5, Layer, IdentityFunction<>>>
      nn;
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(nn.weights<0>())>, LinearArray<int, 8, 5, 5>>);
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(nn.weights<1>())>, LinearArray<int, 5, 24 * 124 * 124>>);

  NeuralNetwork<int, ConvolutionalInputLayer<64, 3>, ConvolutionalLayer<8, 3, 3>,
                SizedLayer<5, Layer, IdentityFunction<>>>
      nn2;
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(nn2.weights<0>())>, LinearArray<int, 8, 3, 3>>);
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(nn2.weights<1>())>, LinearArray<int, 5, 21 * 21 * 24>>);
  (void) nn;
  (void) nn2;
}

TEST(ConvolutionalNeuralNetwork, FeedForward) {
  NeuralNetwork<int, ConvolutionalInputLayer<4, 1>, ConvolutionalLayer<1, 3, 1>, SizedLayer<1, Layer, ReluFunction<>>>
      nn;
  nn.weights<0>() = larray(larray(larray(1, 0, 1), larray(1, 0, 1), larray(1, 0, 1)));
  nn.weights<1>() = larray(larray(1, 1, 1, 1));

  auto in = larray(larray(larray(5, 4, 3, 2), larray(1, 0, 0, 1), larray(2, 2, 2, 2), larray(-2, 4, 3, 1)));
  auto exp = larray(larray(40));
  //  auto rez = nn.feedForward(in);
  //  ASSERT_EQ(exp, rez);
}