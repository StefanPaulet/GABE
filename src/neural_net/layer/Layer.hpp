//
// Created by stefan on 2/15/24.
//

#pragma once

#include "LayerTraits.hpp"
#include "utils/concepts/Concepts.hpp"
#include <functional>
#include <utils/math/function/Function.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

namespace gabe::nn {
template <typename DataType, typename ActivationFunction, typename Dim> class Layer : private ActivationFunction {
public:
  static const Size dimension = Dim::size();

private:
  using InnerLinearArray = gabe::utils::math::LinearColumnArray<DataType, dimension>;

public:
  using LayerFunction = ActivationFunction;
  using OutputType = InnerLinearArray;

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

template <typename DataType, utils::concepts::ContainerFunctionType ActivationFunction, typename Dim>
class Layer<DataType, ActivationFunction, Dim> : private ActivationFunction {
public:
  static const Size dimension = Dim::size();

private:
  using InnerLinearArray = gabe::utils::math::LinearColumnArray<DataType, dimension>;

public:
  using LayerFunction = ActivationFunction;
  using OutputType = InnerLinearArray;

  Layer() = default;
  Layer(Layer const&) = default;
  Layer(Layer&&) noexcept = default;

  auto feedForward(InnerLinearArray const& input) -> InnerLinearArray {
    return (*static_cast<ActivationFunction*>(this))(input);
  }

  auto backPropagate(InnerLinearArray const& input) -> InnerLinearArray {
    return static_cast<ActivationFunction*>(this)->derive(input);
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

  template <typename T> auto randomize_weights(T&& transformer) -> void {
    _weights.transform(std::forward<T>(transformer));
    _biases.transform(std::forward<T>(transformer));
  }

private:
  InnerLinearMatrix _weights {};
  InnerLinearArray _biases {};
};

template <typename DataType, Size inputDepth, Size kernelSize, Size depth,
          gabe::utils::concepts::ConvolutionalLayerPairType DerivedClass>
class ConvolutionalLayerPairContainer {
protected:
  using InnerKernelArray = typename utils::math::LinearArray<DataType, depth, inputDepth, kernelSize, kernelSize>;

public:
  ConvolutionalLayerPairContainer() = default;
  ConvolutionalLayerPairContainer(ConvolutionalLayerPairContainer const&) = default;
  ConvolutionalLayerPairContainer(ConvolutionalLayerPairContainer&&) noexcept = default;

  auto& weights() { return _weights; }

private:
  InnerKernelArray _weights {};
};

template <typename DataType, typename FirstLayer, typename SecondLayer, typename... RemainingLayers> class LayerPair :
    public LayerPairContainer<DataType, NDLType<FirstLayer>::template Type<DataType>::dimension,
                              NDLType<SecondLayer>::template Type<DataType>::dimension,
                              LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...>>,
    public LayerPair<DataType, SecondLayer, RemainingLayers...> {
protected:
  static constexpr auto flDim = NDLType<FirstLayer>::template Type<DataType>::dimension;
  static constexpr auto slDim = NDLType<SecondLayer>::template Type<DataType>::dimension;

  using Input = utils::math::LinearColumnArray<DataType, flDim>;
  using LayerPairContainer =
      LayerPairContainer<DataType, flDim, slDim, LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...>>;
  using NextLayerPair = LayerPair<DataType, SecondLayer, RemainingLayers...>;
  using SecondLayerType = typename NDLType<SecondLayer>::template Type<DataType>;

  using LayerPairContainer::biases;
  using LayerPairContainer::weights;

public:
  template <Size idx> auto& weights() {
    if constexpr (idx == 0) {
      return weights();
    } else {
      return static_cast<NextLayerPair*>(this)->template weights<idx - 1>();
    }
  }

  template <Size idx> auto& biases() {
    if constexpr (idx == 0) {
      return biases();
    } else {
      return static_cast<NextLayerPair*>(this)->template biases<idx - 1>();
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

  template <typename T> auto randomize_weights(T&& transformer) {
    LayerPairContainer::randomize_weights(std::forward<T>(transformer));
    NextLayerPair::randomize_weights(std::forward<T>(transformer));
  }
};

template <typename DataType, typename FirstLayer, typename SecondLayer>
class LayerPair<DataType, FirstLayer, SecondLayer> :
    public LayerPairContainer<DataType, NDLType<FirstLayer>::template Type<DataType>::dimension,
                              NDLType<SecondLayer>::template Type<DataType>::dimension,
                              LayerPair<DataType, FirstLayer, SecondLayer>> {
protected:
  static constexpr auto flDim = NDLType<FirstLayer>::template Type<DataType>::dimension;
  static constexpr auto slDim = NDLType<SecondLayer>::template Type<DataType>::dimension;

  using LayerPairContainer = LayerPairContainer<DataType, flDim, slDim, LayerPair<DataType, FirstLayer, SecondLayer>>;
  using Input = typename utils::math::LinearArray<DataType, flDim, 1>;
  using SecondLayerType = typename NDLType<SecondLayer>::template Type<DataType>;

  using LayerPairContainer::biases;
  using LayerPairContainer::weights;

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

  template <typename T> auto randomize_weights(T&& transformer) {
    LayerPairContainer::randomize_weights(std::forward<T>(transformer));
  }
};

template <typename B, typename D, typename DataType, typename FirstLayer, typename SecondLayer>
class ConvolutionalLayerPairFlattener {
  using Input = typename FirstLayer::template OutputType<DataType>;

public:
  auto feedForward(Input const& input) {
    decltype(std::declval<D>().template weights<0>().transpose()) flattenedInput {input.flatten()};
    return static_cast<D*>(static_cast<B*>(this))->feedForward(flattenedInput);
  }

  template <typename TargetType>
  auto backPropagate(Input const& input, TargetType const& target, DataType learningRate) {
    decltype(std::declval<D>().template weights<0>().transpose()) flattenedInput {input.flatten()};
    auto rez = static_cast<D*>(static_cast<B*>(this))->backPropagate(flattenedInput, target, learningRate);
    return Input {
        static_cast<D*>(static_cast<B*>(this))->backPropagate(flattenedInput, target, learningRate).flatten()};
  }
};

template <typename DataType, gabe::utils::concepts::ThreeDimensionalLayerType FirstLayer, typename SecondLayer,
          typename... RemainingLayers>
class LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...> :
    public LayerPair<DataType, SizedLayer<FirstLayer::dimension, Layer, gabe::utils::math::IdentityFunction<>>,
                     SecondLayer, RemainingLayers...>,
    public ConvolutionalLayerPairFlattener<
        LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...>,
        LayerPair<DataType, SizedLayer<FirstLayer::dimension, Layer, gabe::utils::math::IdentityFunction<>>,
                  SecondLayer, RemainingLayers...>,
        DataType, FirstLayer, SecondLayer> {
private:
  using Flattener = ConvolutionalLayerPairFlattener<
      LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...>,
      LayerPair<DataType, SizedLayer<FirstLayer::dimension, Layer, gabe::utils::math::IdentityFunction<>>, SecondLayer,
                RemainingLayers...>,
      DataType, FirstLayer, SecondLayer>;
  using InnerLayerPair =
      LayerPair<DataType, SizedLayer<FirstLayer::dimension, Layer, gabe::utils::math::IdentityFunction<>>, SecondLayer,
                RemainingLayers...>;

public:
  using Flattener::backPropagate;
  using Flattener::feedForward;
  using InnerLayerPair::biases;
  using InnerLayerPair::weights;
};

template <typename DataType, gabe::utils::concepts::ThreeDimensionalLayerType FirstLayer, typename SecondLayer>
class LayerPair<DataType, FirstLayer, SecondLayer> :
    public LayerPair<DataType, SizedLayer<FirstLayer::dimension, Layer, gabe::utils::math::IdentityFunction<>>,
                     SecondLayer>,
    public ConvolutionalLayerPairFlattener<
        LayerPair<DataType, FirstLayer, SecondLayer>,
        LayerPair<DataType, SizedLayer<FirstLayer::dimension, Layer, gabe::utils::math::IdentityFunction<>>,
                  SecondLayer>,
        DataType, FirstLayer, SecondLayer> {
private:
  using Flattener = ConvolutionalLayerPairFlattener<
      LayerPair<DataType, FirstLayer, SecondLayer>,
      LayerPair<DataType, SizedLayer<FirstLayer::dimension, Layer, gabe::utils::math::IdentityFunction<>>, SecondLayer>,
      DataType, FirstLayer, SecondLayer>;
  using InnerLayerPair =
      LayerPair<DataType, SizedLayer<FirstLayer::dimension, Layer, gabe::utils::math::IdentityFunction<>>, SecondLayer>;

public:
  using Flattener::backPropagate;
  using Flattener::feedForward;
  using InnerLayerPair::biases;
  using InnerLayerPair::weights;
};

template <typename DataType, gabe::utils::concepts::ThreeDimensionalLayerType FirstLayer,
          gabe::utils::concepts::ConvolutionalLayerType SecondLayer, typename... RemainingLayers>
class LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...> :
    public LayerPair<DataType,
                     typename SecondLayer::template Type<DataType, typename FirstLayer::template OutputType<DataType>>,
                     RemainingLayers...>,
    public ConvolutionalLayerPairContainer<
        DataType, FirstLayer::template OutputType<DataType>::size(),
        SecondLayer::template Type<DataType, typename FirstLayer::template OutputType<DataType>>::kernelSize,
        SecondLayer::template Type<DataType, typename FirstLayer::template OutputType<DataType>>::depth,
        LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...>> {
private:
  using SecondLayerType =
      typename SecondLayer::template Type<DataType, typename FirstLayer::template OutputType<DataType>>;
  using Input = typename FirstLayer::template OutputType<DataType>;
  using InnerContainer = ConvolutionalLayerPairContainer<
      DataType, FirstLayer::template OutputType<DataType>::size(),
      SecondLayer::template Type<DataType, typename FirstLayer::template OutputType<DataType>>::kernelSize,
      SecondLayer::template Type<DataType, typename FirstLayer::template OutputType<DataType>>::depth,
      LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...>>;
  using InnerContainer::weights;
  using NextLayerPair = LayerPair<DataType, SecondLayerType, RemainingLayers...>;

public:
  template <Size idx> auto& weights() {
    if constexpr (idx == 0) {
      return weights();
    } else {
      return static_cast<NextLayerPair*>(this)->template weights<idx - 1>();
    }
  }

  auto feedForward(Input const& input) {
    return NextLayerPair::feedForward(SecondLayerType().feedForward(input, weights()));
  }

  template <typename TargetType>
  auto backPropagate(Input const& input, TargetType const& target, DataType learningRate) {
    auto nextLayerGradient =
        NextLayerPair::backPropagate(SecondLayerType().feedForward(input, weights()), target, learningRate);
    auto [kernelGradient, currentLayerGradient] = SecondLayerType().backPropagate(input, weights(), nextLayerGradient);
    weights() -= kernelGradient * learningRate;
    return currentLayerGradient;
  }
};

template <typename DataType, gabe::utils::concepts::ThreeDimensionalLayerType FirstLayer,
          gabe::utils::concepts::PoolingLayerType SecondLayer, typename... RemainingLayers>
class LayerPair<DataType, FirstLayer, SecondLayer, RemainingLayers...> :
    public LayerPair<DataType,
                     typename SecondLayer::template Type<DataType, typename FirstLayer::template OutputType<DataType>>,
                     RemainingLayers...> {
private:
  using SecondLayerType =
      typename SecondLayer::template Type<DataType, typename FirstLayer::template OutputType<DataType>>;
  using Input = typename FirstLayer::template OutputType<DataType>;
  using NextLayerPair = LayerPair<DataType, SecondLayerType, RemainingLayers...>;

public:
  template <Size idx> auto& weights() {
    static_assert(idx != 0, "Cannot require weights for a pooling layer; there aren't any");
    return static_cast<NextLayerPair*>(this)->template weights<idx - 1>();
  }

  auto feedForward(Input const& input) { return NextLayerPair::feedForward(SecondLayerType().feedForward(input)); }

  template <typename TargetType>
  auto backPropagate(Input const& input, TargetType const& target, DataType learningRate) {
    auto nextLayerGradient = NextLayerPair::backPropagate(SecondLayerType().feedForward(input), target, learningRate);
    return SecondLayerType().backPropagate(input, nextLayerGradient);
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
