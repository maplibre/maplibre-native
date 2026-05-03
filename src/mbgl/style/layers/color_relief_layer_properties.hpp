// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/style/layers/color_relief_layer.hpp>
#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/shaders/attributes.hpp>
#include <mbgl/shaders/uniforms.hpp>

namespace mbgl {
namespace style {

struct ColorReliefColor : ColorRampProperty {
};

struct ColorReliefOpacity : PaintProperty<float> {
    static float defaultValue() { return 1.f; }
};

class ColorReliefPaintProperties : public Properties<
    ColorReliefColor,
    ColorReliefOpacity
> {};

class ColorReliefLayerProperties final : public LayerProperties {
public:
    explicit ColorReliefLayerProperties(Immutable<ColorReliefLayer::Impl>);
    ColorReliefLayerProperties(
        Immutable<ColorReliefLayer::Impl>,
        ColorReliefPaintProperties::PossiblyEvaluated);
    ~ColorReliefLayerProperties() override;

    unsigned long constantsMask() const override;

    expression::Dependency getDependencies() const noexcept override;

    const ColorReliefLayer::Impl& layerImpl() const noexcept;
    // Data members.
    ColorReliefPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mbgl

// clang-format on
