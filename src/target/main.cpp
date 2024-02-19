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
using linearArray::lmatrix;

int main() {
  NeuralNetwork<float, NDL<InputLayer, Dimension<3>>, NDL<Layer, IdentityFunction<>, Dimension<3>>> nn;
  auto input = larray(larray(1.0f, 2, 3));
  nn.weights(0) = larray(larray(1.0f, 1, 1), larray(1.0f, 1, 1), larray(1.0f, 1, 1));
  nn.biases(0) = larray(larray(1.0f, 2, 3)).transpose();
  std::cout << nn.feedForward(input.transpose());

  auto x = lmatrix<3, int, int>({1, 2, 3}, {4, 5, 6});
}
