//
// Created by stefan on 2/8/24.
//

#include <iostream>
#include <neural_net/NeuralNetwork.hpp>
#include <neural_net/layer/Layer.hpp>
#include <utils/math/function/Function.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

namespace {
using gabe::Size;
template <Size s, typename L> auto&& weights(L&& layer) noexcept {
  return std::forward<L>(layer).template weights<s>();
}
template <Size s, typename L> auto&& biases(L&& layer) noexcept {
  return std::forward<L>(layer).template biases<s>();
}
} // namespace

int main() {
  //  NeuralNetwork<float, SizedLayer<5, InputLayer>, SizedLayer<3, Layer, IdentityFunction<>>,
  //                SizedLayer<1, Layer, IdentityFunction<>>> nn;
  //  auto input = larray(larray(1.0f, 1, 1, 1, 1));
  //  weights<0>(nn) = larray(larray(1.0f, 1, 1, 1, 1), larray(1.0f, 1, 1, 1, 1), larray(1.0f, 1, 1, 1, 1));
  //  biases<0>(nn) = larray(larray(1.0f, 2, 3)).transpose();
  //  weights<1>(nn) = larray(larray(1.0f), larray(1.0f), larray(1.0f)).transpose();
  //  std::cout << nn.feedForward(input.transpose());
  //
  //  auto x = lmatrix<3, int, int>({1, 2, 3}, {4, 5, 6});
}
