//
// Created by stefan on 2/15/24.
//

#pragma once

#include <types.hpp>
#include <utils/math/function/Function.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

namespace gabe::nn {


template <typename DataType, typename ActivationFunction, Size size> class Layer {
private:
  using InnerLinearArray = gabe::utils::math::LinearArray<DataType, size, 1>;

public:
  static const Size dimension = size;

  Layer() = default;
  Layer(Layer const&) = default;
  Layer(Layer&&) noexcept = default;
  explicit Layer(InnerLinearArray const& neurons) : _neurons(neurons) {}

  template <Size inputSize> auto feedForward(utils::math::LinearArray<DataType, inputSize, 1> const& input)
      -> InnerLinearArray {
    _neurons = input.project(ActivationFunction());
    return _neurons;
  }

  auto const& neurons() const { return _neurons; }
  auto& neurons() { return _neurons; }

private:
  InnerLinearArray _neurons;
};

namespace impl {
template <typename DataType, Size flSize, Size slSize> class LayerPairContainer {
protected:
  using InnerLinearMatrix = typename utils::math::LinearArray<DataType, slSize, flSize>;
  using InnerLinearArray = typename utils::math::LinearColumnArray<DataType, slSize>;

public:
  LayerPairContainer() = default;
  LayerPairContainer(LayerPairContainer const&) = default;
  LayerPairContainer(LayerPairContainer&&) noexcept = default;

  auto& weights() { return _weights; }
  auto const& weights() const { return _weights; }
  auto& biases() { return _biases; }
  auto const& biases() const { return _biases; }

private:
  InnerLinearMatrix _weights {};
  InnerLinearArray _biases {};
};

template <typename DataType, typename FirstLayer, typename SecondLayer, typename... RemainingLayers> class LayerPair :
    public LayerPairContainer<DataType, FirstLayer::dimension, SecondLayer::dimension>,
    public LayerPair<DataType, SecondLayer, RemainingLayers...> {

private:
  using Input = utils::math::LinearArray<DataType, FirstLayer::dimension>;
  using NextLayerPair = LayerPair<DataType, SecondLayer, RemainingLayers...>;
  using InnerLayerPair = LayerPair<DataType, SecondLayer, RemainingLayers...>;
  using LayerPairContainer<DataType, FirstLayer::dimension, SecondLayer::dimension>::biases;
  using LayerPairContainer<DataType, FirstLayer::dimension, SecondLayer::dimension>::weights;

public:
  auto& weights(int idx) {
    if (idx == 0) {
      return weights();
    }
    return static_cast<InnerLayerPair*>(this)->weights(idx - 1);
  }
  auto const& weights(int idx) const {
    if (idx == 0) {
      return weights();
    }
    return static_cast<InnerLayerPair const*>(this)->weights(idx - 1);
  }
  auto& biases(int idx) {
    if (idx == 0) {
      return biases();
    }
    return static_cast<InnerLayerPair*>(this)->biases(idx - 1);
  }
  auto const& biases(int idx) const {
    if (idx == 0) {
      return biases();
    }
    return static_cast<InnerLayerPair const*>(this)->biases(idx - 1);
  }

  auto feedForward(Input const& input) {
    return NextLayerPair::feedForward(
        SecondLayer().template feedForward<FirstLayer::dimension>(weights().product(input) + biases()));
  }
};

template <typename DataType, typename FirstLayer, typename SecondLayer>
class LayerPair<DataType, FirstLayer, SecondLayer> :
    public LayerPairContainer<DataType, FirstLayer::dimension, SecondLayer::dimension> {
private:
  using Input = typename utils::math::LinearArray<DataType, FirstLayer::dimension, 1>;
  using LayerPairContainer<DataType, FirstLayer::dimension, SecondLayer::dimension>::biases;
  using LayerPairContainer<DataType, FirstLayer::dimension, SecondLayer::dimension>::weights;

public:
  auto& weights(int idx) {
    assert(idx == 0 && "Request for weights beyond last layer");
    return weights();
  }
  auto const& weights(int idx) const {
    assert(idx == 0 && "Request for weights beyond last layer");
    return weights();
  }
  auto& biases(int idx) {
    assert(idx == 0 && "Request for biases beyond last layer");
    return biases();
  }
  auto const& biases(int idx) const {
    assert(idx == 0 && "Request for biases beyond last layer");
    return biases();
  }

  auto feedForward(Input const& input) { return SecondLayer().feedForward(weights().product(input) + biases()); }
};
} // namespace impl

namespace layer {
template <typename DataType, Size size> using InputLayer = Layer<DataType, utils::math::IdentityFunction<>, size>;
} // namespace layer
} // namespace gabe::nn