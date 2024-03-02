//
// Created by stefan on 2/15/24.
//

#pragma once

#include "LayerTraits.hpp"
#include <functional>
#include <utils/math/function/Function.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

namespace gabe::nn {
template <typename DataType, typename ActivationFunction, typename Dim> class Layer : private ActivationFunction {
public:
  static const Size dimension = Dim::size;

private:
  using InnerLinearArray = gabe::utils::math::LinearColumnArray<DataType, dimension>;

public:
  using LayerFunction = ActivationFunction;

  Layer() = default;
  Layer(Layer const&) = default;
  Layer(Layer&&) noexcept = default;

  auto feedForward(InnerLinearArray const& input) -> InnerLinearArray {
    return input.project(*static_cast<ActivationFunction*>(this));
  }

  auto backPropagate(InnerLinearArray const& input) -> InnerLinearArray {
    return input.project([this]<typename T>(T&& value) {
      return static_cast<ActivationFunction*>(this)->derive(std::forward<T>(value));
    });
  }
};

template <typename DataType, typename ActivationFunction, typename CostFunction, typename Dim>
class OutputLayer : public Layer<DataType, ActivationFunction, Dim>, private CostFunction {
private:
  using Layer = Layer<DataType, ActivationFunction, Dim>;
  using InnerLinearArray = gabe::utils::math::LinearColumnArray<DataType, Layer::dimension>;
  using Layer::backPropagate;

public:
  using LayerFunction = CostFunction;
  using Layer::feedForward;

  auto backPropagate(InnerLinearArray const& input, InnerLinearArray const& target)
      -> std::pair<InnerLinearArray, InnerLinearArray> {
    return {backPropagate(input), (*static_cast<CostFunction*>(this)).derive(feedForward(input), target)};
  }
};

namespace impl {
template <typename DataType, Size flSize, Size slSize, typename DerivedClass> class LayerPairContainer {
protected:
  using InnerLinearMatrix = typename utils::math::LinearMatrix<DataType, slSize, flSize>;
  using InnerLinearArray = typename utils::math::LinearColumnArray<DataType, slSize>;

public:
  LayerPairContainer() = default;
  LayerPairContainer(LayerPairContainer const&) = default;
  LayerPairContainer(LayerPairContainer&&) noexcept = default;

  auto& weights() { return _weights; }

  auto& biases() { return _biases; }

private:
  InnerLinearMatrix _weights {};
  InnerLinearArray _biases {};
};

template <typename DataType, typename FirstLayer, typename SecondLayer, typename... RemainingLayers> class LayerPair :
    public LayerPairContainer<DataType, NDLType<FirstLayer>::template Type<DataType>::dimension,
                              NDLType<SecondLayer>::template Type<DataType>::dimension,
                              LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...>>,
    public LayerPair<DataType, SecondLayer, RemainingLayers...> {

private:
  static constexpr auto flDim = NDLType<FirstLayer>::template Type<DataType>::dimension;
  static constexpr auto slDim = NDLType<SecondLayer>::template Type<DataType>::dimension;

  using Input = utils::math::LinearColumnArray<DataType, flDim>;
  using NextLayerPair = LayerPair<DataType, SecondLayer, RemainingLayers...>;
  using InnerLayerPair = LayerPair<DataType, SecondLayer, RemainingLayers...>;
  using FirstLayerType = typename NDLType<FirstLayer>::template Type<DataType>;
  using SecondLayerType = typename NDLType<SecondLayer>::template Type<DataType>;

  using LayerPairContainer<DataType, flDim, slDim, LayerPair>::biases;
  using LayerPairContainer<DataType, flDim, slDim, LayerPair>::weights;

public:
  template <Size idx> auto& weights() {
    if constexpr (idx == 0) {
      return weights();
    } else {
      return static_cast<InnerLayerPair*>(this)->template weights<idx - 1>();
    }
  }

  template <Size idx> auto& biases() {
    if constexpr (idx == 0) {
      return biases();
    } else {
      return static_cast<InnerLayerPair*>(this)->template biases<idx - 1>();
    }
  }

  auto feedForward(Input const& input) {
    return NextLayerPair::feedForward(SecondLayerType().feedForward(weights().product(input) + biases()));
  }

  template <typename TargetType>
  auto backPropagate(Input const& input, TargetType const& target, DataType learning_rate) {
    auto z_value = weights().product(input) + biases();
    auto nextLayerGradient =
        NextLayerPair::backPropagate(SecondLayerType().feedForward(z_value), target, learning_rate);
    auto currentLayerGradient = nextLayerGradient * SecondLayerType().backPropagate(z_value);

    auto returnGradient = currentLayerGradient.transpose().product(weights()).transpose();
    biases() -= currentLayerGradient * learning_rate;
    weights() -= currentLayerGradient.product(input.transpose()) * learning_rate;
    return returnGradient;
  }
};

template <typename DataType, typename FirstLayer, typename SecondLayer>
class LayerPair<DataType, FirstLayer, SecondLayer> :
    public LayerPairContainer<DataType, NDLType<FirstLayer>::template Type<DataType>::dimension,
                              NDLType<SecondLayer>::template Type<DataType>::dimension,
                              LayerPair<DataType, FirstLayer, SecondLayer>> {
private:
  static constexpr auto flDim = NDLType<FirstLayer>::template Type<DataType>::dimension;
  static constexpr auto slDim = NDLType<SecondLayer>::template Type<DataType>::dimension;

  using Input = typename utils::math::LinearArray<DataType, flDim, 1>;
  using FirstLayerType = typename NDLType<FirstLayer>::template Type<DataType>;
  using SecondLayerType = typename NDLType<SecondLayer>::template Type<DataType>;

  using LayerPairContainer<DataType, flDim, slDim, LayerPair>::biases;
  using LayerPairContainer<DataType, flDim, slDim, LayerPair>::weights;

public:
  template <Size idx> auto& weights() {
    static_assert(idx == 0, "Request for weights beyond last layer");
    return weights();
  }

  template <Size idx> auto& biases() {
    static_assert(idx == 0, "Request for biases beyond last layer");
    return biases();
  }

  auto feedForward(Input const& input) { return SecondLayerType().feedForward(weights().product(input) + biases()); }

  template <typename TargetType>
  auto backPropagate(Input const& input, TargetType const& target, DataType learning_rate) {
    static_assert(utils::math::impl::is_cost_function<typename SecondLayerType::LayerFunction>::value,
                  "Final layer must have a cost function");
    auto rezPair = SecondLayerType().backPropagate(weights().product(input) + biases(), target);
    auto endLayerGradient = std::get<0>(rezPair) * std::get<1>(rezPair);

    auto returnGradient = endLayerGradient.transpose().product(weights()).transpose();

    biases() -= endLayerGradient * learning_rate;
    weights() -= endLayerGradient.product(input.transpose()) * learning_rate;

    return returnGradient;
  }
};
} // namespace impl

template <typename DataType, typename Dim> struct InputLayer :
    Layer<DataType, utils::math::IdentityFunction<DataType>, Dim> {
  using Layer<DataType, utils::math::IdentityFunction<DataType>, Dim>::Layer;
};

template <Size s, typename N> auto&& weights(N&& neuralNet) noexcept {
  return std::forward<N>(neuralNet).template weights<s>();
}
template <Size s, typename N> auto&& biases(N&& neuralNet) noexcept {
  return std::forward<N>(neuralNet).template biases<s>();
}
} // namespace gabe::nn
