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
  auto x = linearArray::larray(larray(1, 2, 3));
  auto y = linearArray::larray(larray(0, 2, 4));
  x = x.maximize(y, [](int lhs, int rhs) { return lhs < rhs; });
  std::cout << x << '\n';
  return 0;

  auto v =
      loadDelimSeparatedFile<LinearArray<float, 7, 1>>("../test/featuretest/datasets/seeds/seeds_dataset.txt", '\t');

  v.normalize();
  for (auto const& e : v.data()) std::cout << e.data << '\n';
  return 0;
  std::random_device rd;
  std::ranges::shuffle(v.data(), std::mt19937(rd()));

  decltype(v) train {};
  train.data() = {v.data().begin(), v.data().begin() + v.data().size() / 3 * 2};
  decltype(v) validate {};
  validate.data() = {v.data().begin() + v.data().size() / 3 * 2, v.data().end()};

  //  NeuralNetwork<float, SizedLayer<7, InputLayer>, SizedLayer<5, Layer, SigmoidFunction<>>,
  //                SizedLayer<3, OutputLayer, SoftmaxFunction<>, MeanSquaredErrorFunction<>>>
  //      nn;

  NeuralNetwork<float, SizedLayer<7, InputLayer>, SizedLayer<5, Layer, SigmoidFunction<>>,
                SizedLayer<3, Layer, SigmoidFunction<>>,
                SizedLayer<3, OutputLayer, SoftmaxFunction<>, MeanSquaredErrorFunction<>>>
      nn;

  nn.randomize_weights();

  auto sub_one = [](float x) { return x - 1; };
  auto add_one = [](float x) { return x + 1; };

  nn.backPropagate(100, 0.05f, train, OneHotEncoder<short int, LinearArray<float, 3, 1>, decltype(sub_one)>());

  std::cout << nn.validate(validate, SoftMaxDecoder<short int, LinearArray<float, 3, 1>, decltype(add_one)>());

  //  auto train =
  //      loadMNIST<LinearArray<float, 28 * 28, 1>>("../test/featuretest/datasets/mnist/train", MNISTDataSetType::TRAIN);
  //  std::random_device rd;
  //  std::shuffle(train.begin(), train.end(), std::mt19937(rd()));
  //  decltype(train) actual_train = {train.begin(), train.begin() + 100};
  //
  //  auto validate =
  //      loadMNIST<LinearArray<float, 28 * 28, 1>>("../test/featuretest/datasets/mnist/test", MNISTDataSetType::TEST);
  //  decltype(validate) actual_validate = {validate.begin(), validate.begin() + 100};
  //  NeuralNetwork<float, SizedLayer<28 * 28, InputLayer>, SizedLayer<128, Layer, SigmoidFunction<>>,
  //                SizedLayer<10, OutputLayer, SoftmaxFunction<>, MeanSquaredErrorFunction<>>>
  //      nn;
  //
  //  nn.backPropagate(100, 0.05f, actual_train, OneHotEncoder<short int, LinearArray<float, 10, 1>>());
  //  std::cout << nn.validate(actual_validate, SoftMaxDecoder<short int, LinearArray<float, 10, 1>>());
}
