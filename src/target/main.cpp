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
  NeuralNetwork<float, SizedLayer<2, InputLayer>, SizedLayer<2, Layer, IdentityFunction<>>,
                SizedLayer<1, OutputLayer, IdentityFunction<>, MeanSquaredErrorFunction<>>>
      nn;
  auto input = larray(larray(2.f, 3));
  auto target = larray(larray(1.f));
  nn.weights<0>() = larray(larray(.11f, .21f), larray(.12f, .08f));
  nn.weights<1>() = larray(larray(.14f, .15f));
  nn.backPropagate(input.transpose(), target.transpose(), 0.05f);
  nn.biases<0>() = larray(larray(.0f, .0f)).transpose();
  nn.biases<1>() = larray(larray(.0f));
  nn.feedForward(input.transpose());
}
