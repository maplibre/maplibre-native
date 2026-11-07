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
                                        const Point<double>&,
                                        double elevation) const override;
    ProjectedTilePoint drawnTilePoint(const ProjectionData&,
                                      const UnwrappedTileID&,
                                      const Point<double>&,
                                      double elevation) const override;
    double circleRadiusCorrection(const TransformState&) const override;
    double pixelScale(const TransformState&) const override;
    double pitchedTextCorrection(const TransformState&, const Point<double>&, const UnwrappedTileID&) const override;

    /// GL JS `earthRadius`, `GLOBE_RADIUS` in the shaders: the meters an elevation is measured against.
    static constexpr double globeRadiusMeters = 6371008.8;

    /// Globe radius in pixels at the given world size and center latitude, so zoom means the same on both projections.
    static double globeRadiusPixels(double worldSize, double centerLatitude);

    /// GL JS `planetScaleAtLatitude`: the cosine of the center latitude the globe radius is divided by. Mercator ends
    /// at the edge of its range, so past it the globe keeps its size there.
    static double planetScale(double centerLatitude);

    /// GL JS `_getMaxLatitude`: how close to a pole the center can be at the given zoom. Past the Mercator range there
    /// are no tiles, so the center reaches the pole only while that area fits in the view, and eases back to the edge
    /// of the range as the area outgrows the view.
    static double maxLatitude(const TransformState&, double zoom);

    /// The unit-sphere position of a point in tile units.
    static vec3 tileCoordinatesToSphere(const Point<double>& tilePoint, const UnwrappedTileID&);

    /// The view-projection matrix that takes unit-sphere positions to clip space.
    static mat4 globeViewProjectionMatrix(const TransformState&, double radius);

    /// The plane (as ax + by + cz + d) that separates the visible hemisphere from the back of the globe.
    static vec4 clippingPlane(const TransformState&, double radius);

    /// Camera position in unit-sphere space.
    static vec3 cameraPosition(const TransformState&, double radius);

    static vec3 surfaceVector(const LatLng&);
    static LatLng surfaceVectorToLatLng(const vec3&);

    /// Unit-sphere point under a screen pixel (y up, as `TransformState` takes it); pixels off the globe snap to
    /// the nearest point on the horizon.
    static vec3 screenCoordinateToSurface(const TransformState&, const ScreenCoordinate&);
    /// Whether a screen pixel (y up) looks at the globe rather than past it.
    static bool screenCoordinateHitsGlobe(const TransformState&, const ScreenCoordinate&);
    static LatLng screenCoordinateToLatLng(const TransformState&, const ScreenCoordinate&, LatLng::WrapMode);
    static ScreenCoordinate latLngToScreenCoordinate(const TransformState&, const LatLng&, vec4& clip);

    /// The center that puts `latLng` under `anchor` with the bearing unchanged, if one exists.
    static std::optional<LatLng> centerForLocationAtPoint(const TransformState&,
                                                          const LatLng& latLng,
                                                          const ScreenCoordinate& anchor);

    /// Zoom change that keeps the globe the same apparent size when the center moves between latitudes.
    static double zoomAdjustment(double fromLatitude, double toLatitude);

    /// GL JS `computeGlobePanCenter`: the center after a pan of `offset` pixels, turning the planet by degrees per
    /// pixel at the center's latitude. The longitude is unwrapped and the latitude clamped to the poles.
    static LatLng panCenter(const TransformState&, const ScreenCoordinate& offset);

    /// GL JS `handleMapControlsRollPitchBearingZoom`: after the zoom changed by `zoomDelta`, move the center so that
    /// `zoomLocation` stays under `anchor`, exactly where that is well posed and by a damped heuristic near the
    /// horizon, where a pixel is worth degrees of arc and the exact solution jumps or does not exist.
    /// With `freeBearing` the planet turns as one body instead and the bearing follows it, so near a pole, where the
    /// longitude of the center swings, the view does not spin: the exact part is the rotation that brings
    /// `zoomLocation` back under `anchor`, the heuristic part moves the center along the great circle towards it.
    static void zoomAroundPoint(TransformState&,
                                const ScreenCoordinate& anchor,
                                const LatLng& zoomLocation,
                                double zoomDelta,
                                bool freeBearing = false);

    /// GL JS `versorSetLocationAtPoint` with the bearing held, as a drag uses it: the center after the one rotation
    /// that brings the location under `point - panDelta` back under `point` (both y up). Within 12 degrees of the
    /// pole's edge the cursor turns a dial around the pole, and a swing over a pole stops on it. The longitude stays
    /// unwrapped around the current center, the latitude is not yet constrained.
    static LatLng dragCenter(const TransformState&, const ScreenCoordinate& point, const ScreenCoordinate& panDelta);

    struct CenterAndBearing {
        LatLng center;
        /// In degrees, with CameraOptions's sign.
        double bearing;
    };

    /// GL JS `versorSetLocationAtPoint` without the fixed bearing: the whole rotation that brings the location under
    /// `point - panDelta` under `point` (both y up), so the grabbed location tracks the cursor exactly and the bearing
    /// turns with the planet, over the poles too. The longitude stays unwrapped around the current center, the
    /// latitude is not yet constrained.
    static CenterAndBearing dragCamera(const TransformState&,
                                       const ScreenCoordinate& point,
                                       const ScreenCoordinate& panDelta);

    /// Where a fling of `offset` pixels (y up) ends with the bearing free: the planet turns about the axis a drag in
    /// that direction at `center` (y up) turns it about, by GL JS's degrees per pixel at the center's latitude and
    /// short of half a turn, the orientation carried along.
    static CenterAndBearing flingCamera(const TransformState&,
                                        const ScreenCoordinate& center,
                                        const ScreenCoordinate& offset);

    /// The camera `t` of the way from one camera to another, the planet turning about one axis.
    static CenterAndBearing interpolateCamera(const CenterAndBearing& from, const CenterAndBearing& to, double t);

    /// The shorter signed way around from one angle to the other, in degrees.
    static double differenceOfAnglesDegrees(double from, double to);

    /// Distance between two locations along the surface, in pixels at the given world size and center latitude.
    static double surfaceDistancePixels(double worldSize, double centerLatitude, const LatLng&, const LatLng&);

    /// The location `t` of the way from `start` to `start + delta`, the longitude paced at 1 / cos(latitude) so the
    /// globe appears to turn at a steady rate. The pace grows without bound towards the poles, so past the Mercator
    /// range it keeps its value at the edge.
    static LatLng interpolateLatLng(const LatLng& start, double deltaLatitude, double deltaLongitude, double t);
};

} // namespace mln
