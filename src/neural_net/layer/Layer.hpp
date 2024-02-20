//
// Created by stefan on 2/15/24.
//

#pragma once

#include "LayerTraits.hpp"
#include <utils/math/function/Function.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

namespace gabe::nn {
template <typename DataType, typename ActivationFunction, typename Dim> class Layer : private ActivationFunction {
public:
  static const Size dimension = Dim::size;

private:
  using InnerLinearArray = gabe::utils::math::LinearColumnArray<DataType, dimension>;

public:
  Layer() = default;
  Layer(Layer const&) = default;
  Layer(Layer&&) noexcept = default;
  explicit Layer(InnerLinearArray const& neurons) : _neurons(neurons) {}

  auto feedForward(InnerLinearArray const& input) -> InnerLinearArray {
    _neurons = input.project(*static_cast<ActivationFunction*>(this));
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
  using InnerLinearMatrix = typename utils::math::LinearMatrix<DataType, slSize, flSize>;
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
    public LayerPairContainer<DataType, NDLType<FirstLayer>::template Type<DataType>::dimension,
                              NDLType<SecondLayer>::template Type<DataType>::dimension>,
    public LayerPair<DataType, SecondLayer, RemainingLayers...> {

private:
  static constexpr auto flDim = NDLType<FirstLayer>::template Type<DataType>::dimension;
  static constexpr auto slDim = NDLType<SecondLayer>::template Type<DataType>::dimension;

  using Input = utils::math::LinearColumnArray<DataType, flDim>;
  using NextLayerPair = LayerPair<DataType, SecondLayer, RemainingLayers...>;
  using InnerLayerPair = LayerPair<DataType, SecondLayer, RemainingLayers...>;
  using SecondLayerType = typename NDLType<SecondLayer>::template Type<DataType>;

  using LayerPairContainer<DataType, flDim, slDim>::biases;
  using LayerPairContainer<DataType, flDim, slDim>::weights;


public:
  template <Size idx> auto& weights() {
    if constexpr (idx == 0) {
      return weights();
    } else {
      return static_cast<InnerLayerPair*>(this)->template weights<idx - 1>();
    }
  }

  template <Size idx> auto const& weights() const {
    if constexpr (idx == 0) {
      return weights();
    }
    return static_cast<InnerLayerPair const*>(this)->template weights<idx - 1>();
  }

  template <Size idx> auto& biases() {
    if constexpr (idx == 0) {
      return biases();
    } else {
      return static_cast<InnerLayerPair*>(this)->template biases<idx - 1>();
    }
  }

  template <Size idx> auto const& biases() const {
    if constexpr (idx == 0) {
      return biases();
    }
    return static_cast<InnerLayerPair const*>(this)->template biases<idx - 1>();
  }

  auto feedForward(Input const& input) {
    return NextLayerPair::feedForward(SecondLayerType().feedForward(weights().product(input) + biases()));
  }
};

template <typename DataType, typename FirstLayer, typename SecondLayer>
class LayerPair<DataType, FirstLayer, SecondLayer> :
    public LayerPairContainer<DataType, NDLType<FirstLayer>::template Type<DataType>::dimension,
                              NDLType<SecondLayer>::template Type<DataType>::dimension> {
private:
  static constexpr auto flDim = NDLType<FirstLayer>::template Type<DataType>::dimension;
  static constexpr auto slDim = NDLType<SecondLayer>::template Type<DataType>::dimension;

  using Input = typename utils::math::LinearArray<DataType, flDim, 1>;
  using SecondLayerType = typename NDLType<SecondLayer>::template Type<DataType>;


  using LayerPairContainer<DataType, flDim, slDim>::biases;
  using LayerPairContainer<DataType, flDim, slDim>::weights;

public:
  template <Size idx> auto& weights() {
    static_assert(idx == 0, "Request for weights beyond last layer");
    return weights();
  }

  template <Size idx> auto const& weights() const {
    static_assert(idx == 0, "Request for weights beyond last layer");
    return weights();
  }

  template <Size idx> auto& biases() {
    static_assert(idx == 0, "Request for biases beyond last layer");
    return biases();
  }

  template <Size idx> auto const& biases() const {
    static_assert(idx == 0, "Request for biases beyond last layer");
    return biases();
  }

  auto feedForward(Input const& input) { return SecondLayerType().feedForward(weights().product(input) + biases()); }
};
} // namespace impl

template <typename DataType, typename Dim> struct InputLayer :
    Layer<DataType, utils::math::IdentityFunction<DataType>, Dim> {};
} // namespace gabe::nn
