// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mln/style/layers/circle_layer_properties.hpp>

#include <mln/style/layers/circle_layer_impl.hpp>

namespace mln {
namespace style {

CircleLayerProperties::CircleLayerProperties(
    Immutable<CircleLayer::Impl> impl_)
    : LayerProperties(std::move(impl_)) {}

CircleLayerProperties::CircleLayerProperties(
    Immutable<CircleLayer::Impl> impl_,
    CirclePaintProperties::PossiblyEvaluated evaluated_)
  : LayerProperties(std::move(impl_)),
    evaluated(std::move(evaluated_)) {}

CircleLayerProperties::~CircleLayerProperties() = default;

unsigned long CircleLayerProperties::constantsMask() const {
    return evaluated.constantsMask();
}

const CircleLayer::Impl& CircleLayerProperties::layerImpl() const noexcept {
    return static_cast<const CircleLayer::Impl&>(*baseImpl);
}

expression::Dependency CircleLayerProperties::getDependencies() const noexcept {
    return layerImpl().paint.getDependencies() | layerImpl().layout.getDependencies();
}

} // namespace style
} // namespace mln

// clang-format on
