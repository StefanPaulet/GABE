//
// Created by stefan on 2/8/24.
//

#include <neural_net/NeuralNetwork.hpp>
#include <random>
#include <utils/concepts/Concepts.hpp>
#include <utils/data/dataLoader/DataLoader.hpp>
namespace {
using namespace gabe::utils::math;
using namespace gabe::utils::data;
using namespace gabe::nn;
using linearArray::larray;
} // namespace

int main() {

  NeuralNetwork<int, ConvolutionalInputLayer<128, 3>, ConvolutionalLayer<8, 5, 2>,
                SizedLayer<5, Layer, IdentityFunction<>>>
      nn;
  LinearArray<int, 3, 128, 128> in {};
  nn.feedForward(in);
}
