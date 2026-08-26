#pragma once

#include <mln/map/projection_base.hpp>

namespace mln {

/// The globe: tile geometry projected onto a unit sphere, viewed with a vertical perspective camera.
/// Port of MapLibre GL JS `vertical_perspective_transform.ts`.
class VerticalPerspectiveProjection final : public ProjectionBase {
public:
    Point<double> project(const LatLng&, double scale) const override;
    LatLng unproject(const Point<double>&, double scale, LatLng::WrapMode) const override;

    void tileMatrix(mat4&, const UnwrappedTileID&, double scale) const override;

    ProjectionData getProjectionData(const TransformState&,
                                     const UnwrappedTileID&,
                                     const mat4& mercatorMatrix) const override;

    /// Globe radius in pixels at the given world size and center latitude, so zoom means the same on both projections.
    static double globeRadiusPixels(double worldSize, double centerLatitude);

    /// The unit-sphere position of a point in tile units.
    static vec3 tileCoordinatesToSphere(const Point<double>& tilePoint, const UnwrappedTileID&);

    /// The view-projection matrix that takes unit-sphere positions to clip space.
    static mat4 globeViewProjectionMatrix(const TransformState&, double globeRadiusPixels);

    /// The plane (as ax + by + cz + d) that separates the visible hemisphere from the back of the globe.
    static vec4 clippingPlane(const TransformState&, double globeRadiusPixels);
};

} // namespace mln
