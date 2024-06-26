//
// Created by stefan on 2/15/24.
//

#pragma once
#include "initialization/InitializationScheme.hpp"
#include "layer/ConvolutionalLayer.hpp"
#include "layer/Layer.hpp"
#include "utils/data/Data.hpp"
#include <random>

namespace gabe::nn {

template <typename DataType, typename InputLayer, typename... Layers> class NeuralNetwork :
    public impl::LayerPair<DataType, InputLayer, Layers...> {
  template <typename InputLayerType> using ImageDataSet = utils::data::ImageDataSet<InputLayerType>;
  template <typename InputLayerType> using YoloDataSet = utils::data::YoloDataSet<InputLayerType>;

private:
  using LayerPair = impl::LayerPair<DataType, InputLayer, Layers...>;

public:
  using impl::LayerPair<DataType, InputLayer, Layers...>::backPropagate;
  using impl::LayerPair<DataType, InputLayer, Layers...>::feedForward;

  auto randomize_weights(double lower_end, double higher_end) {
    auto transformer = [lower_end, higher_end](DataType) {
      static std::random_device randomDevice {};
      static std::uniform_real_distribution dist(lower_end, higher_end);
      return dist(randomDevice);
    };

    LayerPair::randomize_weights(transformer);
  }

  template <typename Input, typename LabelEncoderType> auto backPropagate(Size epochCount, DataType learningRate,
                                                                          ImageDataSet<Input> const& dataSet,
                                                                          LabelEncoderType&& labelEncoder) -> void {
    for (Size idx = 0; idx < epochCount; ++idx) {
      for (auto const& e : dataSet.data()) {
        backPropagate(e.data, labelEncoder(e.label), learningRate);
      }
    }
  }

  template <typename Input, typename LabelEncoder>
  auto backPropagateWithSerialization(Size epochCount, DataType learningRate, ImageDataSet<Input> const& dataSet,
                                      LabelEncoder&& labelEncoder, std::string const& serializationFile) -> void {
    for (Size idx = 0, propIdx = 0; idx < epochCount; ++idx) {
      for (auto const& e : dataSet.data()) {
        backPropagate(e.data, labelEncoder(e.label), learningRate);
        ++propIdx;
        if (propIdx % 20 == 0) {
          std::cout << "Serialization " << propIdx << " done\n";
          serialize(serializationFile);
        }
      }
    }
  }

  template <typename Input, typename Clipper>
  auto yoloBackPropagateWithSerialization(Size epochCount, DataType learningRate, YoloDataSet<Input> const& dataSet,
                                          std::string const& serializationFile, Clipper&& clipper) -> void {
    for (Size idx = 0, propIdx = 0; idx < epochCount; ++idx) {
      for (auto const& e : dataSet.data()) {
        backPropagate(e.data, e.labels, learningRate, std::forward<Clipper>(clipper));
        ++propIdx;

        if (propIdx % 50 == 0) {
          std::cout << "Serializing inside " + serializationFile + std::to_string(propIdx) + "\n";
          serialize(serializationFile + std::to_string(propIdx));
        }
      }
    }
  }

  template <typename InputLayerType, typename LabelDecoderType>
  auto validate(ImageDataSet<InputLayerType> const& dataSet, LabelDecoderType&& labelDecoder) -> double {
    double error = .0;
    for (auto const& e : dataSet.data()) {
      auto rez = feedForward(e.data);
      std::cout << labelDecoder(rez) << " " << e.label << '\n';
      if (labelDecoder(rez) != e.label) {
        error += static_cast<DataType>(1) / dataSet.data().size();
      }
    }
    return error;
  }

  auto serialize(std::string const& fileName) {
    FILE* out = fopen(fileName.c_str(), "w");
    LayerPair::serialize(out);
    fclose(out);
  }

  auto deserialize(std::string const& fileName) {
    FILE* in = fopen(fileName.c_str(), "r");
    LayerPair::deserialize(in);
    fclose(in);
  }
};
} // namespace gabe::nn
