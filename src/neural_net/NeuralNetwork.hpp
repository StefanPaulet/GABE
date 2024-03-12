//
// Created by stefan on 2/15/24.
//

#pragma once
#include "layer/Layer.hpp"
#include "utils/data/Data.hpp"
#include <random>

namespace gabe::nn {

template <typename DataType, typename InputLayer, typename... Layers> class NeuralNetwork :
    public impl::LayerPair<DataType, InputLayer, Layers...> {
  template <typename InputLayerType> using DataSet = utils::data::DataSet<InputLayerType>;

private:
  using LayerPair = impl::LayerPair<DataType, InputLayer, Layers...>;

public:
  using impl::LayerPair<DataType, InputLayer, Layers...>::backPropagate;
  using impl::LayerPair<DataType, InputLayer, Layers...>::feedForward;

  auto randomize_weights() {
    auto transformer = [](DataType) {
      static std::random_device randomDevice {};
      static std::uniform_real_distribution dist(-0.1, 0.1);
      return dist(randomDevice);
    };

    static_cast<LayerPair*>(this)->randomize_weights(transformer);
  }

  template <typename InputLayerType, typename LabelEncoderType>
  auto backPropagate(Size epochCount, DataType learningRate, DataSet<InputLayerType> const& dataSet,
                     LabelEncoderType&& labelEncoder) -> void {
    for (auto idx = 0; idx < epochCount; ++idx) {
      for (auto const& e : dataSet.data()) {
        backPropagate(e.data, labelEncoder(e.label), learningRate);
      }
    }
  }

  template <typename InputLayerType, typename LabelDecoderType>
  auto validate(DataSet<InputLayerType> const& dataSet, LabelDecoderType&& labelDecoder) -> double {
    double error = .0;
    for (auto const& e : dataSet.data()) {
      auto rez = feedForward(e.data);
      std::cout << rez << " " << e.label << " " << labelDecoder(rez) << '\n';
      if (labelDecoder(rez) != e.label) {
        error += static_cast<DataType>(1) / dataSet.data().size();
      }
    }
    return error;
  }
};
} // namespace gabe::nn
