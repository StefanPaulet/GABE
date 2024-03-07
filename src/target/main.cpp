//
// Created by stefan on 2/8/24.
//

#include <neural_net/NeuralNetwork.hpp>
#include <random>
#include <utils/concepts/Concepts.hpp>
#include <utils/dataLoader/DataLoader.hpp>
namespace {
using namespace gabe::utils::math;
using namespace gabe::utils::dataLoader;
using namespace gabe::nn;
using linearArray::larray;
} // namespace

int main() {
  //  auto v = loadDelimSeparatedFile<LinearArray<float, 7, 1>>("../test/featuretest/datasets/seeds/seeds_dataset.txt", '\t');
  //
  //  std::random_device rd;
  //  std::shuffle(v.begin(), v.end(), std::mt19937(rd()));
  //
  //  decltype(v) train {v.begin(), v.begin() + v.size() / 3 * 2};
  //  decltype(v) validate {v.begin() + v.size() / 3 * 2, v.end()};
  //
  //  NeuralNetwork<float, SizedLayer<7, InputLayer>, SizedLayer<5, Layer, SigmoidFunction<>>,
  //                SizedLayer<3, OutputLayer, SoftmaxFunction<>, MeanSquaredErrorFunction<>>>
  //      nn;
  //
  //  auto sub_one = [](float x) { return x - 1; };
  //  auto add_one = [](float x) { return x + 1; };
  //
  //  nn.backPropagate(100, 0.05f, train, OneHotEncoder<short int, LinearArray<float, 3, 1>, decltype(sub_one)>());
  //
  //  std::cout << nn.validate(validate, SoftMaxDecoder<short int, LinearArray<float, 3, 1>, decltype(add_one)>());

  auto train =
      loadMNIST<LinearArray<float, 28 * 28, 1>>("../test/featuretest/datasets/mnist/train", MNISTDataSet::TRAIN);
  std::random_device rd;
  std::shuffle(train.begin(), train.end(), std::mt19937(rd()));
  decltype(train) actual_train = {train.begin(), train.begin() + 100};

  auto validate =
      loadMNIST<LinearArray<float, 28 * 28, 1>>("../test/featuretest/datasets/mnist/test", MNISTDataSet::TEST);
  decltype(validate) actual_validate = {validate.begin(), validate.begin() + 100};
  NeuralNetwork<float, SizedLayer<28 * 28, InputLayer>, SizedLayer<128, Layer, SigmoidFunction<>>,
                SizedLayer<10, OutputLayer, SoftmaxFunction<>, MeanSquaredErrorFunction<>>>
      nn;

  nn.backPropagate(100, 0.05f, actual_train, OneHotEncoder<short int, LinearArray<float, 10, 1>>());
  std::cout << nn.validate(actual_validate, SoftMaxDecoder<short int, LinearArray<float, 10, 1>>());
}
