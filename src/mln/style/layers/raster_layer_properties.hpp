// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mln/style/types.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/style/layers/raster_layer.hpp>
#include <mln/style/layout_property.hpp>
#include <mln/style/paint_property.hpp>
#include <mln/style/properties.hpp>
#include <mln/shaders/attributes.hpp>
#include <mln/shaders/uniforms.hpp>

namespace mln {
namespace style {

struct RasterBrightnessMax : PaintProperty<float> {
    static float defaultValue() { return 1.f; }
};

struct RasterBrightnessMin : PaintProperty<float> {
    static float defaultValue() { return 0.f; }
};

struct RasterContrast : PaintProperty<float> {
    static float defaultValue() { return 0.f; }
};

struct RasterFadeDuration : PaintProperty<float> {
    static float defaultValue() { return 300.f; }
};

struct RasterHueRotate : PaintProperty<float> {
    static float defaultValue() { return 0.f; }
};

struct RasterOpacity : PaintProperty<float> {
    static float defaultValue() { return 1.f; }
};

struct RasterResampling : PaintProperty<RasterResamplingType> {
    static RasterResamplingType defaultValue() { return RasterResamplingType::Linear; }
};

struct RasterSaturation : PaintProperty<float> {
    static float defaultValue() { return 0.f; }
};

class RasterPaintProperties : public Properties<
    RasterBrightnessMax,
    RasterBrightnessMin,
    RasterContrast,
    RasterFadeDuration,
    RasterHueRotate,
    RasterOpacity,
    RasterResampling,
    RasterSaturation
> {};

class RasterLayerProperties final : public LayerProperties {
public:
    explicit RasterLayerProperties(Immutable<RasterLayer::Impl>);
    RasterLayerProperties(
        Immutable<RasterLayer::Impl>,
        RasterPaintProperties::PossiblyEvaluated);
    ~RasterLayerProperties() override;

    unsigned long constantsMask() const override;

    expression::Dependency getDependencies() const noexcept override;

    const RasterLayer::Impl& layerImpl() const noexcept;
    // Data members.
    RasterPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mln

// clang-format on
