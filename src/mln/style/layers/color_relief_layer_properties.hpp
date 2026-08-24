// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mln/style/types.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/style/layers/color_relief_layer.hpp>
#include <mln/style/layout_property.hpp>
#include <mln/style/paint_property.hpp>
#include <mln/style/properties.hpp>
#include <mln/shaders/attributes.hpp>
#include <mln/shaders/uniforms.hpp>

namespace mln {
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
} // namespace mln

// clang-format on
