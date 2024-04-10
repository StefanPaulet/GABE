//
// Created by stefan on 4/8/24.
//

#pragma once
#include "NeuralNetwork.hpp"

namespace gabe::nn {

class ObjectDetection {
  static constexpr Size outputSize = 7 * 7 * 6;
  using LeakyReluFunction = gabe::utils::math::LeakyReluFunction<>;
  using IdentityFunction = gabe::utils::math::IdentityFunction<>;

public:
  using InitializedObjectRecongnitionNet =
      NeuralNetwork<double, ConvolutionalInputLayer<640, 3>,
                    FullStridedConvolutionalLayer<32, 7, 2, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<64, 3, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<64, 3, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    FullConvolutionalLayer<128, 3, LeakyReluFunction, HeInitialization<>>, MaxPoolLayer<2, 2>,
                    ConvolutionalLayer<1, 1, LeakyReluFunction, HeInitialization<>>,
                    InitSizedLayer<1024, Layer, HeInitialization<>, LeakyReluFunction>,
                    InitSizedLayer<outputSize, OutputLayer, UniformInitialization<-1, 1, 10>, IdentityFunction,
                                   gabe::utils::math::MeanSquaredErrorFunction<>>>;
  using ObjectRecongnitionNet = NeuralNetwork<
      double, ConvolutionalInputLayer<640, 3>, StridedConvolutionalLayer<64, 7, 2, LeakyReluFunction>,
      MaxPoolLayer<2, 2>, StridedConvolutionalLayer<192, 3, 2, LeakyReluFunction>, MaxPoolLayer<2, 2>,
      ConvolutionalLayer<128, 1, LeakyReluFunction>, ConvolutionalLayer<256, 3, LeakyReluFunction>, MaxPoolLayer<2, 2>,
      ConvolutionalLayer<256, 3, LeakyReluFunction>, SizedLayer<2048, Layer, LeakyReluFunction>,
      SizedLayer<outputSize, OutputLayer, IdentityFunction, gabe::utils::math::MeanSquaredErrorFunction<>>>;
};
} // namespace gabe::nn
