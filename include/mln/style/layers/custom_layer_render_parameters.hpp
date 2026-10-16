#pragma once

#include <memory>
#include <array>

namespace mln {

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

    /// Standard projection matrix (nearZ = 1 tile unit).
    /// Use this for 2D/flat custom geometry.
    std::array<double, 16> projectionMatrix;

    /// A 4×4 matrix representing the map view’s current near clip projection state.
    std::array<double, 16> nearClippedProjectionMatrix;

    /// Whether the map is drawn as a globe. The matrices above are then the Mercator fallback the globe blends
    /// towards; geometry meant to sit on the sphere goes through `globeProjectionMatrix` instead.
    bool globe;

    /// 1 on the globe, 0 on Mercator, in between while the projection blends.
    double projectionTransition;

    /// Unit-sphere positions (x towards 90°E, y towards the north pole, z towards null island) to clip space; on
    /// Mercator, the tile matrix of the world.
    std::array<double, 16> globeProjectionMatrix;

    /// The plane behind which sphere positions are on the far side of the globe: `dot(position, xyz) + w < 0`.
    std::array<double, 4> globeClippingPlane;

    CustomLayerRenderParameters(const PaintParameters&);
};

} // namespace style
} // namespace mln
