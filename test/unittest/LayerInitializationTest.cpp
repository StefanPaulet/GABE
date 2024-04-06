//
// Created by stefan on 2/14/24.
//

#include "neural_net/NeuralNetwork.hpp"
#include "gtest/gtest.h"

namespace {
using namespace gabe::utils::math;
using namespace gabe::nn;
using gabe::nn::impl::Dimension;
using gabe::nn::impl::NDL;
using linearArray::larray;
} // namespace

TEST(InitializationSchemeTest, HeInitialization) {
  LinearArray<double, 1, 1, 3, 3> lr;
  HeInitialization<5> he;
  he(lr);
}