// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mln/style/layers/fill_layer_properties.hpp>

#include <mln/style/layers/fill_layer_impl.hpp>

namespace mln {
namespace style {

FillLayerProperties::FillLayerProperties(
    Immutable<FillLayer::Impl> impl_)
    : LayerProperties(std::move(impl_)) {}

FillLayerProperties::FillLayerProperties(
    Immutable<FillLayer::Impl> impl_,
    CrossfadeParameters crossfade_,
    FillPaintProperties::PossiblyEvaluated evaluated_)
  : LayerProperties(std::move(impl_)),
    crossfade(crossfade_),
    evaluated(std::move(evaluated_)) {}

FillLayerProperties::~FillLayerProperties() = default;

unsigned long FillLayerProperties::constantsMask() const {
    return evaluated.constantsMask();
}

const FillLayer::Impl& FillLayerProperties::layerImpl() const noexcept {
    return static_cast<const FillLayer::Impl&>(*baseImpl);
}

expression::Dependency FillLayerProperties::getDependencies() const noexcept {
    return layerImpl().paint.getDependencies() | layerImpl().layout.getDependencies();
}

} // namespace style
} // namespace mln

// clang-format on
