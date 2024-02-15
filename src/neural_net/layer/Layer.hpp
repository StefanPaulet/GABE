//
// Created by stefan on 2/15/24.
//

#pragma once

#include <types.hpp>
#include <utils/math/function/Function.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

namespace gabe::nn {

template <typename DataType, Size size> class LayerContainer {
protected:
  using InnerLinearArray = gabe::utils::math::LinearArray<DataType, size, 1>;

public:
  LayerContainer() = default;
  LayerContainer(LayerContainer const&) = default;
  LayerContainer(LayerContainer&&) noexcept = default;
  explicit LayerContainer(InnerLinearArray const& neurons) : _neurons(neurons) {}

  auto const& neurons() const { return _neurons; }
  auto& neurons() { return _neurons; }

private:
  InnerLinearArray _neurons;
};

template <typename DataType, template <typename> typename ActivationFunction, Size size> class Layer :
    public LayerContainer<DataType, size> {
private:
  using InnerLinearArray = typename LayerContainer<DataType, size>::InnerLinearArray;
  using InnerActivationFunction = ActivationFunction<DataType>;

public:
  static const Size dimension = size;

  Layer() = default;
  Layer(Layer const&) = default;
  Layer(Layer&&) noexcept = default;
  explicit Layer(InnerLinearArray const& neurons) : LayerContainer<DataType, size>(neurons) {}

  template <Size inputSize> auto feedForward(utils::math::LinearArray<DataType, inputSize, 1> const& input)
      -> InnerLinearArray {
    this->neurons() = input.project(InnerActivationFunction());
    return this->neurons();
  }
};

template <typename DataType, typename FirstLayer, typename SecondLayer, typename... RemainingLayers> class LayerPair :
    public LayerPair<DataType, SecondLayer, RemainingLayers...> {

private:
  using InnerLinearMatrix = typename utils::math::LinearArray<DataType, SecondLayer::dimension, FirstLayer::dimension>;
  using InnerLinearArray = typename utils::math::LinearArray<DataType, SecondLayer::dimension>;
  using Input = utils::math::LinearArray<DataType, FirstLayer::dimension>;
  using NextLayerPair = LayerPair<DataType, SecondLayer, RemainingLayers...>;

public:
  auto feedForward(Input const& input) {
    return NextLayerPair::feedForward(
        SecondLayer().template feedForward<FirstLayer::dimension>(_weights.product(input) + _bias));
  }

private:
  InnerLinearMatrix _weights;
  InnerLinearArray _bias;
};

template <typename DataType, typename FirstLayer, typename SecondLayer>
class LayerPair<DataType, FirstLayer, SecondLayer> {
private:
  using InnerLinearMatrix = typename utils::math::LinearMatrix<DataType, SecondLayer::dimension, FirstLayer::dimension>;
  using InnerLinearArray = typename utils::math::LinearArray<DataType, SecondLayer::dimension, 1>;
  using Input = typename utils::math::LinearArray<DataType, FirstLayer::dimension, 1>;

public:
  auto feedForward(Input const& input) { return SecondLayer().feedForward(_weights.product(input) + _bias); }

private:
  InnerLinearMatrix _weights {};
  InnerLinearArray _bias {};
};


namespace layer {
template <typename DataType, Size size> using InputLayer = Layer<DataType, utils::math::IdentityFunction, size>;
} // namespace layer
} // namespace gabe::nn