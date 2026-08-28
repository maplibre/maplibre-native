#pragma once

#include <mln/map/projection_base.hpp>

#include <optional>

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

    ProjectedTilePoint projectTilePoint(const ProjectionData&,
                                        const UnwrappedTileID&,
                                        const Point<double>&) const override;
    double circleRadiusCorrection(const TransformState&) const override;
    double pixelScale(const TransformState&) const override;
    double pitchedTextCorrection(const TransformState&, const Point<double>&, const UnwrappedTileID&) const override;

    /// Globe radius in pixels at the given world size and center latitude, so zoom means the same on both projections.
    static double globeRadiusPixels(double worldSize, double centerLatitude);

    /// The unit-sphere position of a point in tile units.
    static vec3 tileCoordinatesToSphere(const Point<double>& tilePoint, const UnwrappedTileID&);

    /// The view-projection matrix that takes unit-sphere positions to clip space.
    static mat4 globeViewProjectionMatrix(const TransformState&, double globeRadiusPixels);

    /// The plane (as ax + by + cz + d) that separates the visible hemisphere from the back of the globe.
    static vec4 clippingPlane(const TransformState&, double globeRadiusPixels);

    /// Camera position in unit-sphere space.
    static vec3 cameraPosition(const TransformState&, double globeRadiusPixels);

    static vec3 surfaceVector(const LatLng&);
    static LatLng surfaceVectorToLatLng(const vec3&);

    /// Unit-sphere point under a screen pixel (y up, as `TransformState` takes it); pixels off the globe snap to
    /// the nearest point on the horizon.
    static vec3 screenCoordinateToSurface(const TransformState&, const ScreenCoordinate&);
    static LatLng screenCoordinateToLatLng(const TransformState&, const ScreenCoordinate&, LatLng::WrapMode);
    static ScreenCoordinate latLngToScreenCoordinate(const TransformState&, const LatLng&, vec4& clip);

    /// The center that puts `latLng` under `anchor` with the bearing unchanged, if one exists.
    static std::optional<LatLng> centerForLocationAtPoint(const TransformState&,
                                                          const LatLng& latLng,
                                                          const ScreenCoordinate& anchor);

    /// Zoom change that keeps the globe the same apparent size when the center moves between latitudes.
    static double zoomAdjustment(double fromLatitude, double toLatitude);

    /// GL JS `handleMapControlsRollPitchBearingZoom`: after the zoom changed by `zoomDelta`, move the center so that
    /// `zoomLocation` stays under `anchor`, exactly where that is well posed and by a damped heuristic near the
    /// horizon, where a pixel is worth degrees of arc and the exact solution jumps or does not exist.
    static void zoomAroundPoint(TransformState&,
                                const ScreenCoordinate& anchor,
                                const LatLng& zoomLocation,
                                double zoomDelta);

    /// The shorter signed way around from one angle to the other, in degrees.
    static double differenceOfAnglesDegrees(double from, double to);

    /// Distance between two locations along the surface, in pixels at the given world size and center latitude.
    static double surfaceDistancePixels(double worldSize, double centerLatitude, const LatLng&, const LatLng&);

    /// The location `t` of the way from `start` to `start + delta`, the longitude paced at 1 / cos(latitude) so the
    /// globe appears to turn at a steady rate.
    static LatLng interpolateLatLng(const LatLng& start, double deltaLatitude, double deltaLongitude, double t);
};

} // namespace mln
