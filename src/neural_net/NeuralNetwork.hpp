//
// Created by stefan on 2/15/24.
//

#pragma once
#include "layer/Layer.hpp"

namespace gabe::nn {
template <typename DataType, typename InputLayer, typename... Layers> class NeuralNetwork :
    public impl::LayerPair<DataType, InputLayer, Layers...> {};
} // namespace gabe::nn
