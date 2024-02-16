//
// Created by stefan on 2/14/24.
//

#include "neural_net/NeuralNetwork.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
using namespace gabe::nn;
} // namespace

//TEST(NeuralNetwork, Construction) {
//  NeuralNetwork<float, 8, HiddenLayer<sigmoidFunction, 5>, HiddenLayer<reluFunction, 3>,
//                OutputLayer<sigmoidFunction, meanSquaredError, 3>>
//      nn {};
//}
//
//TEST(NeuralNetwork, ForwardPropagating) {
//  NeuralNetwork<float, 8, HiddenLayer<sigmoidFunction, 5>, HiddenLayer<reluFunction, 3>,
//                OutputLayer<sigmoidFunction, meanSquaredError, 3>>
//      nn {};
//  std::cout << nn.feedForward(linearArray::larray(1.0f, 2, 3, 4, 5, 6, 7, 8)) << '\n';
//}
