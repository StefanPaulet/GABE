//
// Created by stefan on 2/8/24.
//

#include <iostream>
#include <neural_net/NeuralNetwork.hpp>
#include <neural_net/layer/Layer.hpp>
#include <utils/math/function/Function.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

using namespace gabe::utils::math;
using namespace gabe::nn;
using namespace layer;
using linearArray::larray;

int main() {
  NeuralNetwork<float, InputLayer<float, 5>, Layer<float, SigmoidFunction<>, 3>> nn;
  auto input = larray(larray(1.0f, 2, 3, 4, 5));
  std::cout << nn.feedForward(input.transpose());
}
