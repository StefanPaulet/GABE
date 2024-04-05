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
  auto x = larray(larray(1, 2, 3), larray(4, 2, 1));
  FILE* myCout = fopen("file.out", "w");
  x.serialize(myCout);
  fclose(myCout);
  myCout = fopen("file.out", "r");
  decltype(x) y = LinearArray<int, 2, 3>::deserialize(myCout);
  std::cout << y << '\n';
  fclose(myCout);
}
