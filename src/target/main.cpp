//
// Created by stefan on 2/8/24.
//

#include <neural_net/NeuralNetwork.hpp>
#include <utils/concepts/Concepts.hpp>
#include <utils/dataLoader/DataLoader.hpp>
namespace {
using namespace gabe::utils::math;
using namespace gabe::utils::dataLoader;
} // namespace
#include <cstdint>


int main() {
  auto v = loadMNIST<LinearArray<float, 28 * 28>>("/mnt/SSD-SATA/Clion/GABE/datasets/mnist/test", MNISTDataSet::TEST);
  std::cout << v[0].data << '\n';
  std::cout << v[0].label << '\n';
}
