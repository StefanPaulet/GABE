//
// Created by stefan on 2/29/24.
//

#include "neural_net/NeuralNetwork.hpp"
#include "utils/data/dataLoader/DataLoader.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
using namespace gabe::nn;
using namespace gabe::utils::data;
using gabe::nn::impl::Dimension;
using gabe::nn::impl::NDL;
using linearArray::larray;
} // namespace

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

TEST(NeuralNetwork, SeedsBackPropagation) {
  auto v = loadDelimSeparatedFile<LinearArray<float, 7, 1>>(
      "../../../test/featuretest/datasets/seeds/seeds_dataset.txt", '\t');

  v.normalize();
  std::random_device rd;
  std::ranges::shuffle(v.data(), std::mt19937(rd()));

  decltype(v) train {};
  train.data() = {v.data().begin(), v.data().begin() + v.data().size() / 3 * 2};
  decltype(v) validate {};
  validate.data() = {v.data().begin() + v.data().size() / 3 * 2, v.data().end()};

  NeuralNetwork<float, SizedLayer<7, InputLayer>, SizedLayer<5, Layer, SigmoidFunction<>>,
                SizedLayer<5, Layer, SigmoidFunction<>>, SizedLayer<3, Layer, SigmoidFunction<>>,
                SizedLayer<3, OutputLayer, SoftmaxFunction<>, MeanSquaredErrorFunction<>>>
      nn;

  nn.randomize_weights(-1.0, 1.0);

  auto sub_one = [](float x) { return x - 1; };
  auto add_one = [](float x) { return x + 1; };

  nn.backPropagate(250, 0.1f, train, OneHotEncoder<short int, LinearArray<float, 3, 1>, decltype(sub_one)>());

  auto err = nn.validate(validate, SoftMaxDecoder<short int, LinearArray<float, 3, 1>, decltype(add_one)>());
  ASSERT_TRUE(err < 0.33);
}

TEST(ConvolutionalNeuralNetwork, SimpleBackPropagate) {
  NeuralNetwork<double, ConvolutionalInputLayer<4, 1>, ConvolutionalLayer<1, 3, IdentityFunction<>>,
                SizedLayer<1, Layer, ReluFunction<>>,
                SizedLayer<1, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  nn.randomize_weights(-2, 2);
  auto in = larray(larray(larray(5.0, 4, 3, 2), larray(1.0, 0, 0, 1), larray(2.0, 2, 2, 2), larray(-2.0, 4, 3, 1)));
  auto target = larray(larray(40.5));
  constexpr auto epochCount = 200;
  for (auto idx = 0; idx < epochCount; ++idx) {
    nn.backPropagate(in, target, 0.05);
  }
  auto learned = nn.feedForward(in);
  ASSERT_EQ(learned, target);
}

TEST(ConvolutionalNeuralNetwork, StridedBackPropagate) {
  NeuralNetwork<double, ConvolutionalInputLayer<4, 1>, StridedConvolutionalLayer<1, 2, 2, IdentityFunction<>>,
                SizedLayer<1, Layer, ReluFunction<>>,
                SizedLayer<1, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  nn.randomize_weights(-5, 5);
  auto in = larray(larray(larray(5.0, 4, 3, 2), larray(1.0, 0, 0, 1), larray(2.0, 2, 2, 2), larray(-2.0, 4, 3, 1)));
  auto target = larray(larray(50.0));
  constexpr auto epochCount = 200;
  for (auto idx = 0; idx < epochCount; ++idx) {
    nn.backPropagate(in, target, 0.05);
  }
  auto learned = nn.feedForward(in);
  ASSERT_EQ(learned, target);
}

TEST(ConvolutionalNeuralNetwork, PoolingBackPropagate) {
  NeuralNetwork<double, ConvolutionalInputLayer<4, 1>, ConvolutionalLayer<1, 3, IdentityFunction<>>, MaxPoolLayer<2, 2>,
                SizedLayer<1, Layer, ReluFunction<>>,
                SizedLayer<1, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  nn.randomize_weights(-1, 1);
  auto in = larray(larray(larray(5.0, 4, 3, 2), larray(1.0, 0, 0, 1), larray(2.0, 2, 2, 2), larray(-2.0, 4, 3, 1)));
  auto target = larray(larray(50.2));
  constexpr auto epochCount = 150;
  for (auto idx = 0; idx < epochCount; ++idx) {
    nn.backPropagate(in, target, 0.05);
  }
  auto learned = nn.feedForward(in);
  ASSERT_EQ(learned, target);
}
