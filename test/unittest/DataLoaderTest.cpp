//
// Created by stefan on 3/4/24.
//

#include "gtest/gtest.h"
#include <utils/data/dataLoader/DataLoader.hpp>
#include <utils/math/linearArray/LinearArray.hpp>
namespace {
using namespace gabe::utils::math;
using namespace gabe::utils::data;
} // namespace

TEST(DataLoader, LoadMNIST) {

  auto array_test =
      loadMNIST<LinearArray<float, 28 * 28>>("../../../test/featuretest/datasets/mnist/test", MNISTDataSetType::TEST);
  auto mtrx_test =
      loadMNIST<LinearArray<float, 28, 28>>("../../../test/featuretest/datasets/mnist/test", MNISTDataSetType::TEST);
  ASSERT_EQ(array_test.data().size(), 10000);
  ASSERT_EQ(mtrx_test.data().size(), 10000);
  static_assert(std::is_same_v<decltype(array_test.data()[0].data), LinearArray<float, 28 * 28>>);
  static_assert(std::is_same_v<decltype(array_test.data()[0].label), float>);
  static_assert(std::is_same_v<decltype(mtrx_test.data()[0].data), LinearArray<float, 28, 28>>);
  static_assert(std::is_same_v<decltype(mtrx_test.data()[0].label), float>);


  auto array_train =
      loadMNIST<LinearArray<float, 28 * 28>>("../../../test/featuretest/datasets/mnist/train", MNISTDataSetType::TRAIN);
  auto mtrx_train =
      loadMNIST<LinearArray<float, 28, 28>>("../../../test/featuretest/datasets/mnist/train", MNISTDataSetType::TRAIN);
  ASSERT_EQ(array_train.data().size(), 60000);
  ASSERT_EQ(mtrx_train.data().size(), 60000);
  static_assert(std::is_same_v<decltype(array_train.data()[0].data), LinearArray<float, 28 * 28>>);
  static_assert(std::is_same_v<decltype(array_train.data()[0].label), float>);
  static_assert(std::is_same_v<decltype(mtrx_train.data()[0].data), LinearArray<float, 28, 28>>);
  static_assert(std::is_same_v<decltype(mtrx_train.data()[0].label), float>);
}

TEST(DataLoader, LoadDelimSeparatedFile) {
  auto dataset =
      loadDelimSeparatedFile<LinearArray<float, 7>>("../../../test/featuretest/datasets/seeds/seeds_dataset.txt", '\t');
  ASSERT_EQ(dataset.data().size(), 210);
  static_assert(std::is_same_v<decltype(dataset.data()[0].data), LinearArray<float, 7>>);
  static_assert(std::is_same_v<decltype(dataset.data()[0].label), float>);
}
