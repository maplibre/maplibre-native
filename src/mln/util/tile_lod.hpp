#pragma once

namespace mln {
namespace util {

/// GL JS `maxMercatorHorizonAngle`, in degrees: the angle from the vertical where the Mercator view puts its horizon.
/// Below 90 so the distance to the horizon stays finite; far enough from 90 that the map loads no needless tiles.
constexpr double maxMercatorHorizonAngleDegrees = 89.25;

/// The zoom each tile is loaded at when the view is pitched: finer for tiles nearer the camera, coarser toward
/// the horizon, under a total tile budget. GL JS `createCalculateTileZoomFunction`.
class TileZoomFunction {
public:
    static constexpr double defaultMaxZoomLevelsOnScreen = 9.314;
    static constexpr double defaultTileCountMaxMinRatio = 3.0;

    /// The terms that depend on the field of view alone are computed once here, for a whole cover.
    explicit TileZoomFunction(double cameraVerticalFOVDegrees,
                              double maxZoomLevelsOnScreen = defaultMaxZoomLevelsOnScreen,
                              double tileCountMaxMinRatio = defaultTileCountMaxMinRatio);

    /// Distances share one unit (GL JS uses Mercator units at zoom 0). Returns an unrounded zoom.
    double operator()(double requestedCenterZoom,
                      double distanceToTile2D,
                      double distanceToTileZ,
                      double distanceToCenter3D) const;

private:
    double cameraVerticalFOV;
    double pitchTileLoadingBehavior;
    double tileCountPitch0;
    double tileCountMaxMinRatio;
};

/// The elevation tiles are culled at: `centerElevation`, growing by up to the tallest feature a tile may hold
/// (`maxContentElevation`, at least 500 m) as the bottom edge of the view nears the horizon. GL JS
/// `getElevationForTileCulling`.
double elevationForTileCulling(double pitchDegrees,
                               double fovDegrees,
                               double centerElevation = 0,
                               double maxContentElevation = 0);

} // namespace util
} // namespace mln
