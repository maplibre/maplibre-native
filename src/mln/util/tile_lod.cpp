#include <mln/util/tile_lod.hpp>

#include <mln/math/angles.hpp>

#include <algorithm>
#include <cmath>

namespace mln {
namespace util {

namespace {

constexpr double assumedMaxFeatureHeightMeters = 500;
constexpr double tileCullingHorizonOnsetDegrees = 15;

double integralOfCosXByP(double p, double x1, double x2) {
    constexpr int numPoints = 10;
    double sum = 0;
    const double dx = (x2 - x1) / numPoints;
    for (int i = 0; i < numPoints; i++) {
        const double x = x1 + (i + 0.5) / numPoints * (x2 - x1);
        sum += dx * std::pow(std::cos(x), p);
    }
    return sum;
}

} // namespace

TileZoomFunction::TileZoomFunction(double cameraVerticalFOVDegrees,
                                   double maxZoomLevelsOnScreen_,
                                   double tileCountMaxMinRatio_)
    : cameraVerticalFOV(cameraVerticalFOVDegrees),
      pitchTileLoadingBehavior(
          2 * ((maxZoomLevelsOnScreen_ - 1) /
                   std::log2(std::cos(deg2rad(maxMercatorHorizonAngleDegrees - cameraVerticalFOVDegrees)) /
                             std::cos(deg2rad(maxMercatorHorizonAngleDegrees))) -
               1)),
      tileCountPitch0(2 * integralOfCosXByP(pitchTileLoadingBehavior - 1, 0, deg2rad(cameraVerticalFOVDegrees / 2))),
      tileCountMaxMinRatio(tileCountMaxMinRatio_) {}

double TileZoomFunction::operator()(double requestedCenterZoom,
                                    double distanceToTile2D,
                                    double distanceToTileZ,
                                    double distanceToCenter3D) const {
    const double centerPitch = std::acos(std::min(1.0, distanceToTileZ / distanceToCenter3D));
    const double highestPitch = std::min(deg2rad(maxMercatorHorizonAngleDegrees),
                                         centerPitch + deg2rad(cameraVerticalFOV / 2));
    const double lowestPitch = std::min(highestPitch, centerPitch - deg2rad(cameraVerticalFOV / 2));
    const double tileCount = integralOfCosXByP(pitchTileLoadingBehavior - 1, lowestPitch, highestPitch);
    const double thisTilePitch = std::atan(distanceToTile2D / distanceToTileZ);
    const double distanceToTile3D = std::hypot(distanceToTile2D, distanceToTileZ);
    double desired = requestedCenterZoom;
    desired += std::log2(distanceToCenter3D / distanceToTile3D /
                         std::max(0.5, std::cos(deg2rad(cameraVerticalFOV / 2))));
    desired += pitchTileLoadingBehavior * std::log2(std::cos(thisTilePitch)) / 2;
    desired -= std::log2(std::max(1.0, tileCount / tileCountPitch0 / tileCountMaxMinRatio)) / 2;
    return desired;
}

double elevationForTileCulling(double pitchDegrees,
                               double fovDegrees,
                               double centerElevation,
                               double maxContentElevation) {
    const double bottomEdgeDegreesAboveHorizontal = maxMercatorHorizonAngleDegrees - pitchDegrees - fovDegrees / 2;
    const double proximityToHorizon = std::clamp(
        (tileCullingHorizonOnsetDegrees - bottomEdgeDegreesAboveHorizontal) / tileCullingHorizonOnsetDegrees, 0.0, 1.0);
    return centerElevation + proximityToHorizon * std::max(assumedMaxFeatureHeightMeters, maxContentElevation);
}

} // namespace util
} // namespace mln
