// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mln/style/layers/line_layer_properties.hpp>

#include <mln/style/layers/line_layer_impl.hpp>

namespace mln {
namespace style {

LineLayerProperties::LineLayerProperties(
    Immutable<LineLayer::Impl> impl_)
    : LayerProperties(std::move(impl_)) {}

LineLayerProperties::LineLayerProperties(
    Immutable<LineLayer::Impl> impl_,
    CrossfadeParameters crossfade_,
    LinePaintProperties::PossiblyEvaluated evaluated_)
  : LayerProperties(std::move(impl_)),
    crossfade(crossfade_),
    evaluated(std::move(evaluated_)) {}

LineLayerProperties::~LineLayerProperties() = default;

unsigned long LineLayerProperties::constantsMask() const {
    return evaluated.constantsMask();
}

const LineLayer::Impl& LineLayerProperties::layerImpl() const noexcept {
    return static_cast<const LineLayer::Impl&>(*baseImpl);
}

expression::Dependency LineLayerProperties::getDependencies() const noexcept {
    return layerImpl().paint.getDependencies() | layerImpl().layout.getDependencies();
}

} // namespace style
} // namespace mln

// clang-format on
