#pragma once

#include <memory>
#include <array>

namespace mbgl {

class PaintParameters;

namespace style {

/**
 * Parameters that define the current camera position for a
 * `CustomLayerHost::render()` function.
 */
struct CustomLayerRenderParameters {
    double width;
    double height;
    double latitude;
    double longitude;
    double zoom;
    double bearing;
    double pitch;
    double fieldOfView;
    std::array<double, 16> projectionMatrix;
    std::array<double, 16> nearClippedProjMatrix;

    CustomLayerRenderParameters(const PaintParameters&);
};

} // namespace style
} // namespace mbgl
