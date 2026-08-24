// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mln/style/types.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/style/layers/background_layer.hpp>
#include <mln/style/layout_property.hpp>
#include <mln/style/paint_property.hpp>
#include <mln/style/properties.hpp>
#include <mln/shaders/attributes.hpp>
#include <mln/shaders/uniforms.hpp>

namespace mln {
namespace style {

struct BackgroundColor : PaintProperty<Color> {
    static Color defaultValue() { return Color::black(); }
};

struct BackgroundOpacity : PaintProperty<float> {
    static float defaultValue() { return 1.f; }
};

struct BackgroundPattern : CrossFadedPaintProperty<expression::Image> {
    static expression::Image defaultValue() { return {}; }
};

class BackgroundPaintProperties : public Properties<
    BackgroundColor,
    BackgroundOpacity,
    BackgroundPattern
> {};

class BackgroundLayerProperties final : public LayerProperties {
public:
    explicit BackgroundLayerProperties(Immutable<BackgroundLayer::Impl>);
    BackgroundLayerProperties(
        Immutable<BackgroundLayer::Impl>,
        CrossfadeParameters,
        BackgroundPaintProperties::PossiblyEvaluated);
    ~BackgroundLayerProperties() override;

    unsigned long constantsMask() const override;

    expression::Dependency getDependencies() const noexcept override;

    const BackgroundLayer::Impl& layerImpl() const noexcept;
    // Data members.
    CrossfadeParameters crossfade;
    BackgroundPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mln

// clang-format on
