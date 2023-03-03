// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/style/properties.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/uniforms.hpp>

namespace mbgl {
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

    const RasterLayer::Impl& layerImpl() const;
    // Data members.
    RasterPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mbgl

// clang-format on
