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
  rlimit rl;
  getrlimit(RLIMIT_STACK, &rl);
  rl.rlim_cur *= 4;
  setrlimit(RLIMIT_STACK, &rl);

  auto train = loadMNIST<LinearArray<double, 1, 28, 28>>("../../../test/featuretest/datasets/mnist/train",
                                                         MNISTDataSetType::TRAIN);
  for (auto& e : train.data()) {
    e.data = e.data / 255;
  }
  std::random_device rd {};
  std::ranges::shuffle(train.data(), std::mt19937(rd()));
    NeuralNetwork<double, ConvolutionalInputLayer<28, 1>, ConvolutionalLayer<32, 3, ReluFunction<>, HeInitialization<>>,
                  MaxPoolLayer<2, 2>, InitSizedLayer<100, Layer, HeInitialization<>, ReluFunction<>>,
                  InitSizedLayer<10, OutputLayer, UniformInitialization<-1, 1, 1000>, SoftmaxFunction<>,
                                 CategoricalCrossEntropyFunction<>>>
        nn;

  nn.serialize("mnistNetwork.out");

  nn.backPropagateWithSerializationWithInfo(
      10, 0.005, train, gabe::utils::math::OneHotEncoder<short int, LinearArray<double, 10, 1>> {}, "mnistNetwork.out",
      std::cout, 20);
  nn.serialize("mnistNetwork.out");
}

TEST(ConvolutionalNeuralNetwork, MNISTValidate) {
  auto totalValidate = loadMNIST<LinearArray<double, 1, 28, 28>>("../../../test/featuretest/datasets/mnist/test",
                                                                 MNISTDataSetType::TEST);
  decltype(totalValidate) validate {{totalValidate.data().begin(), totalValidate.data().begin() + 500}};
  for (auto& e : validate.data()) {
    e.data = e.data / 255;
  }
  NeuralNetwork<double, ConvolutionalInputLayer<28, 1>, ConvolutionalLayer<32, 3, ReluFunction<>>, MaxPoolLayer<2, 2>,
                SizedLayer<100, Layer, ReluFunction<>>,
                SizedLayer<10, OutputLayer, SoftmaxFunction<>, CrossEntropyFunction<>>>
      nn;

  rlimit rl;
  getrlimit(RLIMIT_STACK, &rl);
  rl.rlim_cur *= 4;
  setrlimit(RLIMIT_STACK, &rl);

  nn.deserialize("mnistNetwork.out");
  nn.weights<0>();
  auto err = nn.validate(validate, gabe::utils::math::SoftMaxDecoder<short int, LinearArray<double, 10, 1>> {});
  ASSERT_TRUE(err < 0.5);
}
