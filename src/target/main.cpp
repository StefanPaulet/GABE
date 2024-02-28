//
// Created by stefan on 2/8/24.
//

#include <neural_net/NeuralNetwork.hpp>

namespace {
using namespace gabe::nn;
using namespace gabe::utils::math;
using linearArray::larray;
} // namespace

int main() {
  NeuralNetwork<float, SizedLayer<3, InputLayer>,
                SizedLayer<3, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  auto input = larray(larray(1.0f, 1, 1));
  auto target = larray(larray(2.0f, 0, 0));
  nn.weights<0>() = larray(larray(1.0f, 1, 1), larray(1.0f, 1, 1), larray(1.0f, 1, 1));
  std::cout << nn.biases<0>() << '\n';
  nn.backPropagate(input.transpose(), target.transpose());
  std::cout << nn.biases<0>() << '\n';
}
