// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/debug_layer_properties.hpp>

#include <mbgl/style/layers/debug_layer_impl.hpp>

namespace mbgl {
namespace style {

DebugLayerProperties::DebugLayerProperties(
    Immutable<DebugLayer::Impl> impl_)
    : LayerProperties(std::move(impl_)) {}

DebugLayerProperties::DebugLayerProperties(
    Immutable<DebugLayer::Impl> impl_,
    DebugPaintProperties::PossiblyEvaluated evaluated_)
  : LayerProperties(std::move(impl_)),
    evaluated(std::move(evaluated_)) {}

DebugLayerProperties::~DebugLayerProperties() = default;

unsigned long DebugLayerProperties::constantsMask() const {
    return evaluated.constantsMask();
}

const DebugLayer::Impl& DebugLayerProperties::layerImpl() const {
    return static_cast<const DebugLayer::Impl&>(*baseImpl);
}

} // namespace style
} // namespace mbgl

// clang-format on
