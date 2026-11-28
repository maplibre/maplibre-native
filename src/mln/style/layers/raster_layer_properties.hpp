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
    static constexpr bool SupportsTransition = true;
};

struct RasterBrightnessMin : PaintProperty<float> {
    static float defaultValue() { return 0.f; }
    static constexpr bool SupportsTransition = true;
};

struct RasterContrast : PaintProperty<float> {
    static float defaultValue() { return 0.f; }
    static constexpr bool SupportsTransition = true;
};

struct RasterFadeDuration : PaintProperty<float> {
    static float defaultValue() { return 300.f; }
    static constexpr bool SupportsTransition = false;
};

struct RasterHueRotate : PaintProperty<float> {
    static float defaultValue() { return 0.f; }
    static constexpr bool SupportsTransition = true;
};

struct RasterOpacity : PaintProperty<float> {
    static float defaultValue() { return 1.f; }
    static constexpr bool SupportsTransition = true;
};

struct RasterResampling : PaintProperty<RasterResamplingType> {
    static RasterResamplingType defaultValue() { return RasterResamplingType::Linear; }
    static constexpr bool SupportsTransition = false;
};

struct RasterSaturation : PaintProperty<float> {
    static float defaultValue() { return 0.f; }
    static constexpr bool SupportsTransition = true;
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

    expression::Dependency getEvaluatedDependencies() const noexcept override;

    void collectEvaluatedGlobalStateRefs(std::set<std::string>&) const override;

    const RasterLayer::Impl& layerImpl() const noexcept;
    // Data members.
    RasterPaintProperties::PossiblyEvaluated evaluated;
};

} // namespace style
} // namespace mln

// clang-format on
