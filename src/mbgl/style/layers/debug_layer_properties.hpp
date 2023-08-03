// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/style/layers/debug_layer.hpp>
#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/uniforms.hpp>

namespace mbgl {
namespace style {

struct BorderColor : PaintProperty<Color> {
    static Color defaultValue() { return { 1, 0, 0, 1 }; }
};

class DebugPaintProperties : public Properties<
    BorderColor
> {};

class DebugLayerProperties final : public LayerProperties {
public:
    explicit DebugLayerProperties(Immutable<DebugLayer::Impl>);
    DebugLayerProperties(
        Immutable<DebugLayer::Impl>,
        DebugPaintProperties::PossiblyEvaluated);
    ~DebugLayerProperties() override;

    unsigned long constantsMask() const override;

    const DebugLayer::Impl& layerImpl() const;
    // Data members.
    DebugPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mbgl

// clang-format on
