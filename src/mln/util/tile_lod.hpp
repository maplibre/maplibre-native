#pragma once

namespace mln {
namespace util {

/// The pitch at which Mercator's horizon reaches the top of the screen, in degrees.
constexpr double maxMercatorHorizonAngleDegrees = 89.25;

/// The zoom each tile is loaded at when the view is pitched: finer for tiles nearer the camera, coarser toward
/// the horizon, under a total tile budget. GL JS `createCalculateTileZoomFunction`, which GL JS applies to both
/// projections (the Mercator cover uses it whenever terrain is present or the pitch passes 78.5 - fov/2 degrees).
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

/// Near the bottom of a steeply pitched view, tiles are culled with room for the tallest feature they might hold,
/// so buildings do not vanish before their tile leaves the screen. GL JS `getElevationForTileCulling`:
/// `centerElevation` is the terrain height under the map center and `maxContentElevation` the tallest content the
/// style is known to hold; both are zero without terrain.
double elevationForTileCulling(double pitchDegrees,
                               double fovDegrees,
                               double centerElevation = 0,
                               double maxContentElevation = 0);

} // namespace util
} // namespace mln
