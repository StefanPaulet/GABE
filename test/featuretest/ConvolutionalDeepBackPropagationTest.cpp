//
// Created by stefan on 2/29/24.
//

#include "neural_net/NeuralNetwork.hpp"
#include "utils/data/dataLoader/DataLoader.hpp"
#include "gtest/gtest.h"
#include <sys/resource.h>

namespace {
using namespace gabe::utils::math;
using namespace gabe::nn;
using namespace gabe::utils::data;
using gabe::nn::impl::Dimension;
using gabe::nn::impl::NDL;
using linearArray::larray;
} // namespace

TEST(ConvolutionalNeuralNetwork, SimpleBackPropagate) {
  NeuralNetwork<double, ConvolutionalInputLayer<4, 1>, ConvolutionalLayer<1, 3, IdentityFunction<>>,
                SizedLayer<1, Layer, ReluFunction<>>,
                SizedLayer<1, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  nn.randomize_weights(0, 0.1);
  auto in = larray(larray(larray(5.0, 4, 3, 2), larray(1.0, 0, 0, 1), larray(2.0, 2, 2, 2), larray(-2.0, 4, 3, 1)));
  auto target = larray(larray(20.5));
  constexpr auto epochCount = 250;
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
  nn.randomize_weights(-1, 1);
  auto in = larray(larray(larray(5.0, 4, 3, 2), larray(1.0, 0, 0, 1), larray(2.0, 2, 2, 2), larray(-2.0, 4, 3, 1)));
  auto target = larray(larray(50.0));
  constexpr auto epochCount = 200;
  for (auto idx = 0; idx < epochCount; ++idx) {
    nn.backPropagate(in, target, 0.075);
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
  constexpr auto epochCount = 200;
  for (auto idx = 0; idx < epochCount; ++idx) {
    nn.backPropagate(in, target, 0.075);
  }
  auto learned = nn.feedForward(in);
  ASSERT_EQ(learned, target);
}


TEST(ConvolutionalNeuralNetwork, PropagationWithSerialization) {
  NeuralNetwork<double, ConvolutionalInputLayer<4, 1>, ConvolutionalLayer<1, 3, IdentityFunction<>>, MaxPoolLayer<2, 2>,
                SizedLayer<1, Layer, ReluFunction<>>,
                SizedLayer<1, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  nn.randomize_weights(-1, 1);
  auto in = larray(larray(larray(5.0, 4, 3, 2), larray(1.0, 0, 0, 1), larray(2.0, 2, 2, 2), larray(-2.0, 4, 3, 1)));
  auto target = larray(larray(50.2));
  constexpr auto epochCount = 200;
  for (auto idx = 0; idx < epochCount; ++idx) {
    nn.backPropagate(in, target, 0.075);
  }
  nn.serialize("file.out");
  decltype(nn) nn1;
  nn1.deserialize("file.out");
  auto learned = nn1.feedForward(in);
  ASSERT_EQ(learned, target);
}

TEST(ConvolutionalNeuralNetwork, MNISTPropagation) {
  auto train = loadMNIST<LinearArray<float, 1, 28, 28>>("../../../test/featuretest/datasets/mnist/train",
                                                        MNISTDataSetType::TRAIN);
  train.normalize();

  std::random_device rd {};
  std::ranges::shuffle(train.data(), std::mt19937(rd()));
  NeuralNetwork<float, ConvolutionalInputLayer<28, 1>, ConvolutionalLayer<32, 3, ReluFunction<>>, MaxPoolLayer<2, 2>,
                SizedLayer<100, Layer, ReluFunction<>>,
                SizedLayer<10, OutputLayer, SoftmaxFunction<>, MeanSquaredErrorFunction<>>>
      nn;

  rlimit rl;
  getrlimit(RLIMIT_STACK, &rl);
  rl.rlim_cur *= 4;
  setrlimit(RLIMIT_STACK, &rl);

  nn.backPropagate(10, 0.075f, train, gabe::utils::math::OneHotEncoder<short int, LinearArray<float, 10, 1>> {});
  nn.serialize("mnistNetwork.out");
}

TEST(Smth, smth) {
  NeuralNetwork<float, ConvolutionalInputLayer<28, 1>, ConvolutionalLayer<8, 3, ReluFunction<>>, MaxPoolLayer<2, 2>,
                StridedConvolutionalLayer<12, 3, 2, ReluFunction<>>, MaxPoolLayer<2, 2>,
                SizedLayer<10, Layer, ReluFunction<>>,
                SizedLayer<10, OutputLayer, SoftmaxFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  nn.deserialize("file.out");
  std::cout << nn.weights<0>() << '\n';
}