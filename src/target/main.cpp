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

int main() {
  auto v =
      loadDelimSeparatedFile<LinearArray<float, 7>>("/mnt/SSD-SATA/Clion/GABE/datasets/seeds/seeds_dataset.txt", '\t');
  std::cout << v[0].data << '\n';
  std::cout << v[0].label << '\n';
}
