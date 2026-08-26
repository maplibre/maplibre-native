#include <mln/map/vertical_perspective_projection.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/math/angles.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/projection.hpp>

#include <cmath>
#include <numbers>

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
    const uint64_t tileScale = 1ull << tileID.canonical.z;
    const double s = Projection::worldSize(scale) / tileScale;

    matrix::identity(matrix);
    matrix::translate(matrix,
                      matrix,
                      static_cast<int64_t>(tileID.canonical.x + tileID.wrap * static_cast<int64_t>(tileScale)) * s,
                      static_cast<int64_t>(tileID.canonical.y) * s,
                      0);
    matrix::scale(matrix, matrix, s / util::EXTENT, s / util::EXTENT, 1);
}

double VerticalPerspectiveProjection::globeRadiusPixels(double worldSize, double centerLatitude) {
    return worldSize / (2.0 * std::numbers::pi) / std::cos(util::deg2rad(centerLatitude));
}

vec3 VerticalPerspectiveProjection::tileCoordinatesToSphere(const Point<double>& tilePoint,
                                                            const UnwrappedTileID& tileID) {
    const double scale = 1.0 / static_cast<double>(1ull << tileID.canonical.z);
    const double mercatorX = tilePoint.x / util::EXTENT * scale + tileID.canonical.x * scale;
    const double mercatorY = tilePoint.y / util::EXTENT * scale + tileID.canonical.y * scale;
    const double sphericalX = std::fmod(mercatorX * std::numbers::pi * 2.0 + std::numbers::pi, std::numbers::pi * 2.0);
    const double sphericalY = 2.0 * std::atan(std::exp(std::numbers::pi - (mercatorY * std::numbers::pi * 2.0))) -
                              std::numbers::pi * 0.5;
    const double len = std::cos(sphericalY);
    return {{std::sin(sphericalX) * len, std::sin(sphericalY), std::cos(sphericalX) * len}};
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

ProjectionData VerticalPerspectiveProjection::getProjectionData(const TransformState& state,
                                                                const UnwrappedTileID& tileID,
                                                                const mat4& mercatorMatrix) const {
    const double radius = globeRadiusPixels(Projection::worldSize(state.getScale()), state.getLatLng().latitude());
    const double tileScale = static_cast<double>(1ull << tileID.canonical.z);
    return {.mainMatrix = globeViewProjectionMatrix(state, radius),
            .tileMercatorCoords = {{tileID.canonical.x / tileScale,
                                    tileID.canonical.y / tileScale,
                                    1.0 / tileScale / util::EXTENT,
                                    1.0 / tileScale / util::EXTENT}},
            .clippingPlane = clippingPlane(state, radius),
            .projectionTransition = state.getProjectionTransition(),
            .fallbackMatrix = mercatorMatrix,
            .clipAntimeridian = tileID.canonical.z == 0};
}

} // namespace mln
