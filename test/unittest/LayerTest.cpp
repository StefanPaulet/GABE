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
}
