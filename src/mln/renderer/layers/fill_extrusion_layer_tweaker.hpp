#pragma once

#include <mln/renderer/layer_tweaker.hpp>
#include <mln/style/property_value.hpp>
#include <mln/util/color.hpp>

#include <algorithm>
#include <string>

namespace mln {

/// Whether `fill-extrusion-color` is an expression over `["elevation"]` (height above the ground,
/// in meters) without feature data, which the renderer samples into one lookup texture per layer and
/// applies per pixel. With feature data, see `style::expression::elevationInterpolationStops`.
inline bool usesElevationColorRamp(const style::PropertyValue<Color>& color) {
    return color.isExpression() && color.asExpression().isFeatureConstant() &&
           color.asExpression().getExpression().has(style::expression::Dependency::Elevation);
}

/// The ramp has two rows, so that a ramp which also depends on zoom (`["zoom"]` outermost) is only
/// re-sampled when the zoom moves to other stops, and blended between them on the GPU in between,
/// the same way zoom-and-feature expressions are.
struct ElevationColorRampZoom {
    /// Zoom levels the two rows are sampled at
    Range<float> rows{0, 0};
    /// Blend from the first row to the second
    float t = 0;
    /// Identifies the zoom range the rows are valid for
    Range<float> stops{0, 0};
    bool belowFirstStop = false;

    bool sameRows(const ElevationColorRampZoom& other) const {
        return stops.min == other.stops.min && stops.max == other.stops.max && belowFirstStop == other.belowFirstStop;
    }
};

inline ElevationColorRampZoom elevationColorRampZoom(const style::PropertyExpression<Color>& color, float zoom) {
    if (color.isZoomConstant()) {
        return {};
    }
    const auto stops = color.getCoveringStops(zoom, zoom);
    if (color.getZoomCurve().template is<const style::expression::Step*>()) {
        // Constant between stops: both rows at the current zoom, re-sampled when it crosses a stop
        return {.rows = {zoom, zoom}, .t = 0, .stops = stops, .belowFirstStop = zoom < stops.min};
    }
    const float t = stops.max > stops.min ? std::clamp(color.interpolationFactor(stops, zoom), 0.0f, 1.0f) : 0.0f;
    return {.rows = stops, .t = t, .stops = stops};
}

/**
    Fill extrusion layer specific tweaker
 */
class FillExtrusionLayerTweaker : public LayerTweaker {
public:
    FillExtrusionLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~FillExtrusionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
    gfx::UniformBufferPtr tilePropsUniformBuffer;
#endif
};

} // namespace mln
