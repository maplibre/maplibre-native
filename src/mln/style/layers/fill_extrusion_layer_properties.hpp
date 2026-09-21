// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mln/style/types.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/style/layers/fill_extrusion_layer.hpp>
#include <mln/style/layout_property.hpp>
#include <mln/style/paint_property.hpp>
#include <mln/style/properties.hpp>
#include <mln/shaders/attributes.hpp>
#include <mln/shaders/uniforms.hpp>

namespace mln {
namespace style {

struct FillExtrusionRoundedCornerDistance : LayoutProperty<float> {
    static constexpr const char *name() { return "fill-extrusion-rounded-corner-distance"; }
    static float defaultValue() { return 0.f; }
};

struct FillExtrusionBase : DataDrivenPaintProperty<float, attributes::base, uniforms::base> {
    static float defaultValue() { return 0.f; }
};

struct FillExtrusionColor : DataDrivenPaintProperty<Color, attributes::color, uniforms::color> {
    static Color defaultValue() { return Color::black(); }
};

struct FillExtrusionHeight : DataDrivenPaintProperty<float, attributes::height, uniforms::height> {
    static float defaultValue() { return 0.f; }
};

struct FillExtrusionOpacity : PaintProperty<float> {
    static float defaultValue() { return 1.f; }
};

struct FillExtrusionPattern : CrossFadedDataDrivenPaintProperty<expression::Image, attributes::pattern_to, uniforms::pattern_to, attributes::pattern_from, uniforms::pattern_from> {
    static expression::Image defaultValue() { return {}; }
};

struct FillExtrusionTranslate : PaintProperty<std::array<float, 2>> {
    static std::array<float, 2> defaultValue() { return {{0.f, 0.f}}; }
};

struct FillExtrusionTranslateAnchor : PaintProperty<TranslateAnchorType> {
    static TranslateAnchorType defaultValue() { return TranslateAnchorType::Map; }
};

struct FillExtrusionVerticalGradient : PaintProperty<bool> {
    static bool defaultValue() { return true; }
};

class FillExtrusionLayoutProperties : public Properties<
    FillExtrusionRoundedCornerDistance
> {};

class FillExtrusionPaintProperties : public Properties<
    FillExtrusionBase,
    FillExtrusionColor,
    FillExtrusionHeight,
    FillExtrusionOpacity,
    FillExtrusionPattern,
    FillExtrusionTranslate,
    FillExtrusionTranslateAnchor,
    FillExtrusionVerticalGradient
> {};

class FillExtrusionLayerProperties final : public LayerProperties {
public:
    explicit FillExtrusionLayerProperties(Immutable<FillExtrusionLayer::Impl>);
    FillExtrusionLayerProperties(
        Immutable<FillExtrusionLayer::Impl>,
        CrossfadeParameters,
        FillExtrusionPaintProperties::PossiblyEvaluated);
    ~FillExtrusionLayerProperties() override;

    unsigned long constantsMask() const override;

    expression::Dependency getDependencies() const noexcept override;

    const FillExtrusionLayer::Impl& layerImpl() const noexcept;
    // Data members.
    CrossfadeParameters crossfade;
    FillExtrusionPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mln

// clang-format on
