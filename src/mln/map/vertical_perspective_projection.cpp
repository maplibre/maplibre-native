#include <mln/map/vertical_perspective_projection.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/math/angles.hpp>
#include <mln/math/wrap.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/projection.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>
#include <tuple>

namespace mln {

namespace {

vec3 rotateX(const vec3& v, double rad) {
    const double c = std::cos(rad);
    const double s = std::sin(rad);
    return {{v[0], v[1] * c - v[2] * s, v[1] * s + v[2] * c}};
}

vec3 rotateY(const vec3& v, double rad) {
    const double c = std::cos(rad);
    const double s = std::sin(rad);
    return {{v[2] * s + v[0] * c, v[1], v[2] * c - v[0] * s}};
}

vec3 rotateZ(const vec3& v, double rad) {
    const double c = std::cos(rad);
    const double s = std::sin(rad);
    return {{v[0] * c - v[1] * s, v[0] * s + v[1] * c, v[2]}};
}

double dot(const vec3& a, const vec3& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

double length(const vec3& v) {
    return std::sqrt(dot(v, v));
}

vec3 normalized(const vec3& v) {
    const double len = length(v);
    return {{v[0] / len, v[1] / len, v[2] / len}};
}

vec3 scaled(const vec3& v, double s) {
    return {{v[0] * s, v[1] * s, v[2] * s}};
}

vec3 add(const vec3& a, const vec3& b) {
    return {{a[0] + b[0], a[1] + b[1], a[2] + b[2]}};
}

struct RaySphereIntersection {
    double tMin;
    double tMax;
};

// GL JS `raySphereIntersection`: the numerically stable form for a unit sphere at the origin.
std::optional<RaySphereIntersection> raySphereIntersection(const vec3& origin, const vec3& direction) {
    const double originDotDirection = dot(origin, direction);
    const vec3 inner = add(origin, scaled(direction, -originDotDirection));
    const double discriminant = 1.0 - dot(inner, inner);
    if (discriminant < 0) {
        return std::nullopt;
    }
    const double c = dot(origin, origin) - 1.0;
    const double q = -originDotDirection + (originDotDirection < 0 ? 1 : -1) * std::sqrt(discriminant);
    const double t0 = c / q;
    const double t1 = q;
    return RaySphereIntersection{std::min(t0, t1), std::max(t0, t1)};
}

double pointPlaneSignedDistance(const vec4& plane, const vec3& point) {
    return plane[0] * point[0] + plane[1] * point[1] + plane[2] * point[2] + plane[3];
}

vec3 rayDirectionFromPixel(const TransformState& state,
                           const ScreenCoordinate& point,
                           const mat4& inverseViewProjection) {
    const Size size = state.getSize();
    vec4 clip = {{point.x / size.width * 2.0 - 1.0, point.y / size.height * 2.0 - 1.0, 1.0, 1.0}};
    vec4 world;
    matrix::transformMat4(world, clip, inverseViewProjection);
    const vec3 target = {{world[0] / world[3], world[1] / world[3], world[2] / world[3]}};
    return normalized(add(target, scaled(state.getGlobeCameraPosition(), -1.0)));
}

// Angle to rotate the vector (x0, y0) onto the direction of (x1, y1).
double angleToRotateBetweenVectors2D(double x0, double y0, double x1, double y1) {
    const double len0 = std::sqrt(x0 * x0 + y0 * y0);
    const double len1 = std::sqrt(x1 * x1 + y1 * y1);
    x0 /= len0;
    y0 /= len0;
    x1 /= len1;
    y1 /= len1;
    const double cross = x0 * y1 - x1 * y0;
    const double angle = std::acos(std::clamp(x0 * x1 + y0 * y1, -1.0, 1.0));
    return cross > 0 ? angle : -angle;
}

double distanceOfAnglesRadians(double a, double b) {
    const double twoPi = 2.0 * std::numbers::pi;
    const double d = std::fmod(std::fmod(a - b, twoPi) + twoPi, twoPi);
    return std::min(d, twoPi - d);
}

// The integral of 1 / cos(x): where the longitude gets to when the globe turns at a steady rate.
double integrateSecant(double x) {
    const double half = 0.5 * x;
    const double sin = std::sin(half);
    const double cos = std::cos(half);
    return std::log(sin + cos) - std::log(cos - sin);
}

int sign(double value) {
    return (value > 0) - (value < 0);
}

} // namespace

Point<double> VerticalPerspectiveProjection::project(const LatLng& latLng, double scale) const {
    return Projection::project(latLng, scale);
}

LatLng VerticalPerspectiveProjection::unproject(const Point<double>& point,
                                                double scale,
                                                LatLng::WrapMode wrapMode) const {
    return Projection::unproject(point, scale, wrapMode);
}

void VerticalPerspectiveProjection::tileMatrix(mat4& matrix, const UnwrappedTileID& tileID, double scale) const {
    mercatorTileMatrix(matrix, tileID, scale);
}

double VerticalPerspectiveProjection::globeRadiusPixels(double worldSize, double centerLatitude) {
    return worldSize / (2.0 * std::numbers::pi) / std::cos(util::deg2rad(centerLatitude));
}

vec3 VerticalPerspectiveProjection::tileCoordinatesToSphere(const Point<double>& tilePoint,
                                                            const UnwrappedTileID& tileID) {
    const double scale = 1.0 / static_cast<double>(1ull << tileID.canonical.z);
    const double mercatorX = tilePoint.x / util::EXTENT * scale + tileID.canonical.x * scale;
    const double mercatorY = tilePoint.y / util::EXTENT * scale + tileID.canonical.y * scale;
    const double sphericalX = mercatorX * std::numbers::pi * 2.0 + std::numbers::pi;
    // sin/cos of the latitude from the Mercator Y through the tangent half-angle identities, as the shaders do.
    const double t = std::exp(std::numbers::pi - (mercatorY * std::numbers::pi * 2.0));
    const double t2 = t * t;
    const double sinY = (t2 - 1.0) / (t2 + 1.0);
    const double cosY = (2.0 * t) / (t2 + 1.0);
    return {{std::sin(sphericalX) * cosY, sinY, std::cos(sphericalX) * cosY}};
}

mat4 VerticalPerspectiveProjection::globeViewProjectionMatrix(const TransformState& state, double radius) {
    const Size size = state.getSize();
    const double cameraToCenterDistance = state.getCameraToCenterDistance();
    const LatLng center = state.getLatLng();

    mat4 matrix;
    matrix::perspective(matrix,
                        state.getFieldOfView(),
                        static_cast<double>(size.width) / size.height,
                        0.5,
                        cameraToCenterDistance + radius * 2.0);

    const ScreenCoordinate offset = state.getCenterOffset();
    matrix[8] = -offset.x * 2.0 / size.width;
    matrix[9] = offset.y * 2.0 / size.height;

    if (state.getNorthOrientation() != NorthOrientation::Upwards) {
        matrix::rotate_z(matrix, matrix, -state.getNorthOrientationAngle());
    }

    matrix::translate(matrix, matrix, 0, 0, -cameraToCenterDistance);
    matrix::rotate_z(matrix, matrix, state.getRoll());
    matrix::rotate_x(matrix, matrix, -state.getPitch());
    matrix::rotate_z(matrix, matrix, -state.getBearing());
    matrix::translate(matrix, matrix, 0, 0, -radius);
    matrix::rotate_x(matrix, matrix, util::deg2rad(center.latitude()));
    matrix::rotate_y(matrix, matrix, -util::deg2rad(center.longitude()));
    matrix::scale(matrix, matrix, radius, radius, radius);
    return matrix;
}

vec4 VerticalPerspectiveProjection::clippingPlane(const TransformState& state, double radius) {
    const double pitch = state.getPitch();
    const double distanceCameraToB = state.getCameraToCenterDistance() / radius;
    const double distanceCameraToA = std::sin(pitch) * distanceCameraToB;
    const double distanceAtoC = std::cos(pitch) * distanceCameraToB + 1.0;
    const double distanceCameraToC = std::sqrt(distanceCameraToA * distanceCameraToA + distanceAtoC * distanceAtoC);
    const double tangentPlaneDistanceToC = 1.0 / distanceCameraToC;

    vec3 plane = {{0.0, -distanceCameraToA / distanceCameraToC, distanceAtoC / distanceCameraToC}};

    const LatLng center = state.getLatLng();
    plane = rotateZ(plane, state.getBearing());
    plane = rotateX(plane, -util::deg2rad(center.latitude()));
    plane = rotateY(plane, util::deg2rad(center.longitude()));

    const double scale = 1.0 / std::sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
    return {{plane[0] * scale, plane[1] * scale, plane[2] * scale, -tangentPlaneDistanceToC * scale}};
}

vec3 VerticalPerspectiveProjection::cameraPosition(const TransformState& state, double radius) {
    const LatLng center = state.getLatLng();
    vec3 position = {{0.0, 0.0, state.getCameraToCenterDistance() / radius}};
    position = rotateZ(position, -state.getRoll());
    position = rotateX(position, state.getPitch());
    position = rotateZ(position, state.getBearing());
    position = add(position, {{0.0, 0.0, 1.0}});
    position = rotateX(position, -util::deg2rad(center.latitude()));
    position = rotateY(position, util::deg2rad(center.longitude()));
    return position;
}

vec3 VerticalPerspectiveProjection::surfaceVector(const LatLng& latLng) {
    const double lng = util::deg2rad(latLng.longitude());
    const double lat = util::deg2rad(latLng.latitude());
    const double len = std::cos(lat);
    return {{std::sin(lng) * len, std::sin(lat), std::cos(lng) * len}};
}

LatLng VerticalPerspectiveProjection::surfaceVectorToLatLng(const vec3& surface) {
    const double latitude = util::rad2deg(std::asin(std::clamp(surface[1], -1.0, 1.0)));
    const double lengthXZ = std::sqrt(surface[0] * surface[0] + surface[2] * surface[2]);
    if (lengthXZ <= 1e-6) {
        return {latitude, 0.0};
    }
    const double acosZ = std::acos(std::clamp(surface[2] / lengthXZ, -1.0, 1.0));
    const double longitude = util::rad2deg(surface[0] > 0 ? acosZ : -acosZ);
    return LatLng{latitude, longitude, LatLng::Wrapped};
}

vec3 VerticalPerspectiveProjection::screenCoordinateToSurface(const TransformState& state,
                                                              const ScreenCoordinate& point) {
    const vec3 origin = state.getGlobeCameraPosition();
    const vec3 direction = rayDirectionFromPixel(state, point, state.getInverseGlobeViewProjectionMatrix());

    if (const auto intersection = raySphereIntersection(origin, direction)) {
        return normalized(add(origin, scaled(direction, intersection->tMin)));
    }

    // The ray misses the globe: take the nearest point on the horizon, the circle where the clipping plane cuts the
    // sphere.
    const vec4& plane = state.getGlobeClippingPlane();
    const double directionDotPlane = plane[0] * direction[0] + plane[1] * direction[1] + plane[2] * direction[2];
    const double originToPlane = pointPlaneSignedDistance(plane, origin);
    const double distanceToIntersection = -originToPlane / directionDotPlane;

    vec3 planeIntersection;
    if (distanceToIntersection > 0) {
        planeIntersection = add(origin, scaled(direction, distanceToIntersection));
    } else {
        constexpr double maxRayLength = 2.0;
        const vec3 distantPoint = add(origin, scaled(direction, maxRayLength));
        const double distanceFromPlane = pointPlaneSignedDistance(plane, distantPoint);
        planeIntersection = add(distantPoint, scaled({{plane[0], plane[1], plane[2]}}, -distanceFromPlane));
    }

    const vec3 horizonCenter = scaled({{plane[0], plane[1], plane[2]}}, -plane[3]);
    const double horizonRadius = std::sqrt(std::max(0.0, 1.0 - plane[3] * plane[3]));
    const vec3 relative = add(planeIntersection, scaled(horizonCenter, -1.0));
    return add(horizonCenter, scaled(relative, horizonRadius / length(relative)));
}

LatLng VerticalPerspectiveProjection::screenCoordinateToLatLng(const TransformState& state,
                                                               const ScreenCoordinate& point,
                                                               LatLng::WrapMode wrapMode) {
    const LatLng latLng = surfaceVectorToLatLng(screenCoordinateToSurface(state, point));
    return wrapMode == LatLng::Wrapped ? latLng.wrapped() : latLng;
}

ScreenCoordinate VerticalPerspectiveProjection::latLngToScreenCoordinate(const TransformState& state,
                                                                         const LatLng& latLng,
                                                                         vec4& clip) {
    const vec3 surface = surfaceVector(latLng);
    const vec4 position = {{surface[0], surface[1], surface[2], 1.0}};
    matrix::transformMat4(clip, position, state.getGlobeViewProjectionMatrix());
    const Size size = state.getSize();
    return {(clip[0] / clip[3] * 0.5 + 0.5) * size.width, (clip[1] / clip[3] * 0.5 + 0.5) * size.height};
}

std::optional<LatLng> VerticalPerspectiveProjection::centerForLocationAtPoint(const TransformState& state,
                                                                              const LatLng& latLng,
                                                                              const ScreenCoordinate& anchor) {
    // GL JS `setLocationAtPoint`: solve the rotation that carries the surface point under the pixel onto the target.
    const LatLng center = state.getLatLng();
    const vec3 vecToPixelCurrent = screenCoordinateToSurface(state, anchor);
    const vec3 vecToTarget = surfaceVector(latLng);

    vec3 rotatedPixelVector = rotateY(vecToPixelCurrent, -util::deg2rad(center.longitude()));
    rotatedPixelVector = rotateX(rotatedPixelVector, util::deg2rad(center.latitude()));

    const double vecToTargetXZLengthSquared = vecToTarget[0] * vecToTarget[0] + vecToTarget[2] * vecToTarget[2];
    const double targetXSquared = rotatedPixelVector[0] * rotatedPixelVector[0];
    if (vecToTargetXZLengthSquared < targetXSquared) {
        return std::nullopt;
    }

    const double intersectionA = std::sqrt(vecToTargetXZLengthSquared - targetXSquared);
    const double intersectionB = -intersectionA;

    const auto solve = [&](double intersection) {
        const double lng = angleToRotateBetweenVectors2D(
            vecToTarget[0], vecToTarget[2], rotatedPixelVector[0], intersection);
        const vec3 vecToTargetLng = rotateY(vecToTarget, -lng);
        const double lat = angleToRotateBetweenVectors2D(
            vecToTargetLng[1], vecToTargetLng[2], rotatedPixelVector[1], rotatedPixelVector[2]);
        return std::pair<double, double>{lng, lat};
    };
    const auto [lngA, latA] = solve(intersectionA);
    const auto [lngB, latB] = solve(intersectionB);

    constexpr double limit = std::numbers::pi * 0.5;
    const bool validA = latA >= -limit && latA <= limit;
    const bool validB = latB >= -limit && latB <= limit;

    double lng = 0;
    double lat = 0;
    if (validA && validB) {
        const double centerLng = util::deg2rad(center.longitude());
        const double centerLat = util::deg2rad(center.latitude());
        const double distanceA = distanceOfAnglesRadians(lngA, centerLng) + distanceOfAnglesRadians(latA, centerLat);
        const double distanceB = distanceOfAnglesRadians(lngB, centerLng) + distanceOfAnglesRadians(latB, centerLat);
        std::tie(lng, lat) = distanceA < distanceB ? std::pair{lngA, latA} : std::pair{lngB, latB};
    } else if (validA) {
        std::tie(lng, lat) = std::pair{lngA, latA};
    } else if (validB) {
        std::tie(lng, lat) = std::pair{lngB, latB};
    } else {
        return std::nullopt;
    }
    return LatLng{std::clamp(util::rad2deg(lat), -90.0, 90.0), util::rad2deg(lng)};
}

namespace {
// GL JS vertical_perspective_camera_helper constants.
constexpr double RAY_SURFACE_DISTANCE_FOR_SLOWING_START = 0.3;
constexpr double SLOWING_MULTIPLIER = 0.5;
constexpr double INTERPOLATE_TO_HEURISTIC_START_LNG = 45.0;
constexpr double INTERPOLATE_TO_HEURISTIC_END_LNG = 85.0;
constexpr double INTERPOLATE_TO_HEURISTIC_EXPONENT = 0.25;
constexpr double INTERPOLATE_TO_HEURISTIC_START_HORIZON = 0.95;
constexpr double INTERPOLATE_TO_HEURISTIC_END_HORIZON = 0.999;
constexpr double SLOWING_RADIUS_START = 0.9;
constexpr double SLOWING_RADIUS_STOP = 0.5;
constexpr double SLOWING_RADIUS_SLOW_FACTOR = 0.25;

double remapSaturate(double value, double oldMin, double oldMax, double newMin, double newMax) {
    const double t = std::clamp((value - oldMin) / (oldMax - oldMin), 0.0, 1.0);
    return newMin + (newMax - newMin) * t;
}
} // namespace

void VerticalPerspectiveProjection::zoomAroundPoint(TransformState& state,
                                                    const ScreenCoordinate& anchor,
                                                    const LatLng& zoomLocation,
                                                    double zoomDelta) {
    if (zoomDelta == 0.0) {
        return;
    }
    const LatLng center = state.getLatLng(LatLng::Unwrapped);
    const double dLngRaw = differenceOfAnglesDegrees(center.longitude(), zoomLocation.longitude());
    const double dLng = dLngRaw / (std::abs(dLngRaw / 180.0) + 1.0);
    const double dLat = differenceOfAnglesDegrees(center.latitude(), zoomLocation.latitude());

    // Slow the movement down when the anchor's ray passes far from the planet.
    const vec3 rayOrigin = state.getGlobeCameraPosition();
    const vec3 rayDirection = rayDirectionFromPixel(state, anchor, state.getInverseGlobeViewProjectionMatrix());
    const vec3 closestPoint = add(rayOrigin, scaled(rayDirection, -dot(rayOrigin, rayDirection)));
    const double rayDistanceFromGlobeCenter = length(closestPoint);
    const double distanceFactor = std::exp(
        -std::max(rayDistanceFromGlobeCenter - 1.0 - RAY_SURFACE_DISTANCE_FOR_SLOWING_START, 0.0) * SLOWING_MULTIPLIER);
    const double interpolationFactorHorizon = remapSaturate(rayDistanceFromGlobeCenter,
                                                            INTERPOLATE_TO_HEURISTIC_START_HORIZON,
                                                            INTERPOLATE_TO_HEURISTIC_END_HORIZON,
                                                            0.0,
                                                            1.0);

    // And when the globe is small on the viewport, where that is a stand-in for the anchor being near the horizon.
    const Size size = state.getSize();
    const double radius = globeRadiusPixels(Projection::worldSize(state.getScale()), center.latitude()) /
                          std::min(size.width, size.height);
    const double radiusFactor = remapSaturate(
        radius, SLOWING_RADIUS_START, SLOWING_RADIUS_STOP, 1.0, SLOWING_RADIUS_SLOW_FACTOR);
    const double slowingFactor = std::min(distanceFactor, 1.0 + (radiusFactor - 1.0) * interpolationFactorHorizon);

    const double factor = (1.0 - std::pow(2.0, -zoomDelta)) * slowingFactor;
    const LatLng heuristicCenter{std::clamp(center.latitude() + dLat * factor, -util::LATITUDE_MAX, util::LATITUDE_MAX),
                                 center.longitude() + dLng * factor};

    const LatLng exactCenter = centerForLocationAtPoint(state, zoomLocation, anchor).value_or(center);

    const double interpolationFactorLongitude = remapSaturate(
        std::abs(dLngRaw), INTERPOLATE_TO_HEURISTIC_START_LNG, INTERPOLATE_TO_HEURISTIC_END_LNG, 0.0, 1.0);
    const double heuristicFactor = std::pow(std::max(interpolationFactorLongitude, interpolationFactorHorizon),
                                            INTERPOLATE_TO_HEURISTIC_EXPONENT);
    const LatLng target{
        exactCenter.latitude() +
            differenceOfAnglesDegrees(exactCenter.latitude(), heuristicCenter.latitude()) * heuristicFactor,
        exactCenter.longitude() +
            differenceOfAnglesDegrees(exactCenter.longitude(), heuristicCenter.longitude()) * heuristicFactor};
    const LatLng constrained = state.constrainedCenter(target);
    state.setLatLngZoom(constrained, state.getZoom() + zoomAdjustment(center.latitude(), constrained.latitude()));
}

double VerticalPerspectiveProjection::zoomAdjustment(double fromLatitude, double toLatitude) {
    return std::log2(std::cos(util::deg2rad(toLatitude)) / std::cos(util::deg2rad(fromLatitude)));
}

double VerticalPerspectiveProjection::differenceOfAnglesDegrees(double from, double to) {
    const double a = util::wrap(from, 0.0, 360.0);
    const double b = util::wrap(to, 0.0, 360.0);
    const double direct = b - a;
    const double around = b > a ? direct - 360.0 : direct + 360.0;
    return std::abs(direct) < std::abs(around) ? direct : around;
}

double VerticalPerspectiveProjection::surfaceDistancePixels(double worldSize,
                                                            double centerLatitude,
                                                            const LatLng& a,
                                                            const LatLng& b) {
    const double radians = std::acos(std::clamp(dot(surfaceVector(a), surfaceVector(b)), -1.0, 1.0));
    return radians * globeRadiusPixels(worldSize, centerLatitude);
}

LatLng VerticalPerspectiveProjection::interpolateLatLng(const LatLng& start,
                                                        double deltaLatitude,
                                                        double deltaLongitude,
                                                        double t) {
    const double latitude = start.latitude() + deltaLatitude * t;
    if (std::abs(deltaLatitude) <= 1.0) {
        return {latitude, start.longitude() + deltaLongitude * t};
    }
    const double endLatitude = start.latitude() + deltaLatitude;
    const bool onDifferentHemispheres = sign(endLatitude) != sign(start.latitude());
    const double sampleStart = util::deg2rad(onDifferentHemispheres ? -std::abs(start.latitude())
                                                                    : std::abs(start.latitude()));
    const double sampleEnd = util::deg2rad(std::abs(endLatitude));
    const double valueStart = integrateSecant(sampleStart);
    const double valueEnd = integrateSecant(sampleEnd);
    const double valueT = integrateSecant(sampleStart + t * (sampleEnd - sampleStart));
    const double longitudeT = (valueT - valueStart) / (valueEnd - valueStart);
    return {latitude, start.longitude() + deltaLongitude * longitudeT};
}

ProjectionData VerticalPerspectiveProjection::getProjectionData(const TransformState& state,
                                                                const UnwrappedTileID& tileID,
                                                                const mat4& mercatorMatrix) const {
    return {.mainMatrix = state.getGlobeViewProjectionMatrix(),
            .tileMercatorCoords = mercatorTileCoords(tileID),
            .clippingPlane = state.getGlobeClippingPlane(),
            .projectionTransition = state.getProjectionTransition(),
            .fallbackMatrix = mercatorMatrix};
}

ProjectedTilePoint VerticalPerspectiveProjection::projectTilePoint(const ProjectionData& data,
                                                                   const UnwrappedTileID& tileID,
                                                                   const Point<double>& point) const {
    const vec3 sphere = tileCoordinatesToSphere(point, tileID);
    vec4 pos = {{sphere[0], sphere[1], sphere[2], 1}};
    matrix::transformMat4(pos, pos, data.mainMatrix);
    const auto& plane = data.clippingPlane;
    const double side = plane[0] * sphere[0] + plane[1] * sphere[1] + plane[2] * sphere[2] + plane[3];
    return {.point = {pos[0] / pos[3], pos[1] / pos[3]}, .signedDistanceFromCamera = pos[3], .occluded = side < 0.0};
}

double VerticalPerspectiveProjection::circleRadiusCorrection(const TransformState& state) const {
    return std::cos(util::deg2rad(state.getLatLng().latitude()));
}

double VerticalPerspectiveProjection::pixelScale(const TransformState& state) const {
    return 1.0 / std::cos(util::deg2rad(state.getLatLng().latitude()));
}

double VerticalPerspectiveProjection::pitchedTextCorrection(const TransformState& state,
                                                            const Point<double>& tileAnchor,
                                                            const UnwrappedTileID& tileID) const {
    const double scale = 1.0 / static_cast<double>(1ull << tileID.canonical.z);
    const double mercatorY = tileAnchor.y / util::EXTENT * scale + tileID.canonical.y * scale;
    const double latitude = 2.0 * std::atan(std::exp(std::numbers::pi - (mercatorY * std::numbers::pi * 2.0))) -
                            std::numbers::pi * 0.5;
    return circleRadiusCorrection(state) / std::cos(latitude);
}

} // namespace mln
