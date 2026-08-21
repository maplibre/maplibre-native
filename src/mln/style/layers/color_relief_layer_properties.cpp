// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mln/style/layers/color_relief_layer_properties.hpp>

#include <mln/style/layers/color_relief_layer_impl.hpp>

namespace mln {
namespace style {

ColorReliefLayerProperties::ColorReliefLayerProperties(
    Immutable<ColorReliefLayer::Impl> impl_)
    : LayerProperties(std::move(impl_)) {}

ColorReliefLayerProperties::ColorReliefLayerProperties(
    Immutable<ColorReliefLayer::Impl> impl_,
    ColorReliefPaintProperties::PossiblyEvaluated evaluated_)
  : LayerProperties(std::move(impl_)),
    evaluated(std::move(evaluated_)) {}

ColorReliefLayerProperties::~ColorReliefLayerProperties() = default;

unsigned long ColorReliefLayerProperties::constantsMask() const {
    return evaluated.constantsMask();
}

const ColorReliefLayer::Impl& ColorReliefLayerProperties::layerImpl() const noexcept {
    return static_cast<const ColorReliefLayer::Impl&>(*baseImpl);
}

expression::Dependency ColorReliefLayerProperties::getDependencies() const noexcept {
    return layerImpl().paint.getDependencies();
}

} // namespace style
} // namespace mln

// clang-format on
