// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mln/style/layers/symbol_layer_properties.hpp>

#include <mln/style/layers/symbol_layer_impl.hpp>

namespace mln {
namespace style {

SymbolLayerProperties::SymbolLayerProperties(
    Immutable<SymbolLayer::Impl> impl_)
    : LayerProperties(std::move(impl_)) {}

SymbolLayerProperties::SymbolLayerProperties(
    Immutable<SymbolLayer::Impl> impl_,
    SymbolPaintProperties::PossiblyEvaluated evaluated_)
  : LayerProperties(std::move(impl_)),
    evaluated(std::move(evaluated_)) {}

SymbolLayerProperties::~SymbolLayerProperties() = default;

unsigned long SymbolLayerProperties::constantsMask() const {
    return evaluated.constantsMask();
}

const SymbolLayer::Impl& SymbolLayerProperties::layerImpl() const noexcept {
    return static_cast<const SymbolLayer::Impl&>(*baseImpl);
}

expression::Dependency SymbolLayerProperties::getDependencies() const noexcept {
    return layerImpl().paint.getDependencies() | layerImpl().layout.getDependencies();
}

} // namespace style
} // namespace mln

// clang-format on
