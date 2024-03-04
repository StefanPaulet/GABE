//
// Created by stefan on 3/4/24.
//

#include "gtest/gtest.h"
#include <utils/dataLoader/DataLoader.hpp>
#include <utils/math/linearArray/LinearArray.hpp>
namespace {
using namespace gabe::utils::math;
using namespace gabe::utils::dataLoader;
} // namespace


TEST(DataLoader, LoadMNIST) {

  auto array_test =
      loadMNIST<LinearArray<float, 28 * 28>>("/mnt/SSD-SATA/Clion/GABE/datasets/mnist/test", MNISTDataSet::TEST);
  auto mtrx_test =
      loadMNIST<LinearArray<float, 28, 28>>("/mnt/SSD-SATA/Clion/GABE/datasets/mnist/test", MNISTDataSet::TEST);
  ASSERT_EQ(array_test.size(), 10000);
  ASSERT_EQ(mtrx_test.size(), 10000);
  static_assert(std::is_same_v<decltype(array_test[0].data), LinearArray<float, 28 * 28>>);
  static_assert(std::is_same_v<decltype(array_test[0].label), float>);
  static_assert(std::is_same_v<decltype(mtrx_test[0].data), LinearArray<float, 28, 28>>);
  static_assert(std::is_same_v<decltype(mtrx_test[0].label), float>);


  auto array_train =
      loadMNIST<LinearArray<float, 28 * 28>>("/mnt/SSD-SATA/Clion/GABE/datasets/mnist/train", MNISTDataSet::TRAIN);
  auto mtrx_train =
      loadMNIST<LinearArray<float, 28, 28>>("/mnt/SSD-SATA/Clion/GABE/datasets/mnist/train", MNISTDataSet::TRAIN);
  ASSERT_EQ(array_train.size(), 60000);
  ASSERT_EQ(mtrx_train.size(), 60000);
  static_assert(std::is_same_v<decltype(array_train[0].data), LinearArray<float, 28 * 28>>);
  static_assert(std::is_same_v<decltype(array_train[0].label), float>);
  static_assert(std::is_same_v<decltype(mtrx_train[0].data), LinearArray<float, 28, 28>>);
  static_assert(std::is_same_v<decltype(mtrx_train[0].label), float>);
}

TEST(DataLoader, LoadDelimSeparatedFile) {
  auto dataset =
      loadDelimSeparatedFile<LinearArray<float, 7>>("/mnt/SSD-SATA/Clion/GABE/datasets/seeds/seeds_dataset.txt", '\t');
  ASSERT_EQ(dataset.size(), 210);
  static_assert(std::is_same_v<decltype(dataset[0].data), LinearArray<float, 7>>);
  static_assert(std::is_same_v<decltype(dataset[0].label), float>);
}
