//
// Created by stefan on 2/21/24.
//

#include "neural_net/layer/Layer.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::nn;
using namespace gabe::utils::math;
using gabe::nn::impl::Dimension;
using linearArray::larray;
} // namespace

TEST(LayerTest, Construction) {
  Layer<int, IdentityFunction<>, Dimension<3>> lay1 {};
  SizedLayer<3, Layer, IdentityFunction<>>::template Type<int> lay2;
  static_assert(std::is_same_v<decltype(lay2), decltype(lay1)>);

  Layer<int, IdentityFunction<>, Dimension<3>> lay3 {larray(larray(1, 2, 3)).transpose()};
}

TEST(LayerTest, Neurons) {
  SizedLayer<3, InputLayer>::template Type<int> lay1;
  auto neur1 = larray(larray(1, 2, 3)).transpose();
  lay1.neurons() = neur1;
  ASSERT_EQ(lay1.neurons(), neur1);

  SizedLayer<3, InputLayer>::template Type<int> const lay2 {larray(larray(2, 4, 6)).transpose()};
  auto neur2 = larray(larray(2, 4, 6)).transpose();
  ASSERT_EQ(lay2.neurons(), neur2);
}
