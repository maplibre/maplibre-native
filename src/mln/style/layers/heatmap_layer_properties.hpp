// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mln/style/types.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/style/layers/heatmap_layer.hpp>
#include <mln/style/layout_property.hpp>
#include <mln/style/paint_property.hpp>
#include <mln/style/properties.hpp>
#include <mln/shaders/attributes.hpp>
#include <mln/shaders/uniforms.hpp>

namespace mln {
namespace style {

struct HeatmapColor : ColorRampProperty {
};

struct HeatmapIntensity : PaintProperty<float> {
    static float defaultValue() { return 1.f; }
};

struct HeatmapOpacity : PaintProperty<float> {
    static float defaultValue() { return 1.f; }
};

struct HeatmapRadius : DataDrivenPaintProperty<float, attributes::radius, uniforms::radius> {
    static float defaultValue() { return 30.f; }
};

struct HeatmapWeight : DataDrivenPaintProperty<float, attributes::weight, uniforms::weight> {
    static float defaultValue() { return 1.f; }
};

class HeatmapPaintProperties : public Properties<
    HeatmapColor,
    HeatmapIntensity,
    HeatmapOpacity,
    HeatmapRadius,
    HeatmapWeight
> {};

class HeatmapLayerProperties final : public LayerProperties {
public:
    explicit HeatmapLayerProperties(Immutable<HeatmapLayer::Impl>);
    HeatmapLayerProperties(
        Immutable<HeatmapLayer::Impl>,
        HeatmapPaintProperties::PossiblyEvaluated);
    ~HeatmapLayerProperties() override;

    unsigned long constantsMask() const override;

    expression::Dependency getDependencies() const noexcept override;

    const HeatmapLayer::Impl& layerImpl() const noexcept;
    // Data members.
    HeatmapPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mln

// clang-format on
