#include <mln/map/mercator_projection.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/projection.hpp>

namespace mln {

Point<double> MercatorProjection::project(const LatLng& latLng, double scale) const {
    return Projection::project(latLng, scale);
}

LatLng MercatorProjection::unproject(const Point<double>& point, double scale, LatLng::WrapMode wrapMode) const {
    return Projection::unproject(point, scale, wrapMode);
}

void mercatorTileMatrix(mat4& matrix, const UnwrappedTileID& tileID, double scale) {
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

vec4 mercatorTileCoords(const UnwrappedTileID& tileID) {
    const double tileScale = static_cast<double>(1ull << tileID.canonical.z);
    return {{tileID.canonical.x / tileScale,
             tileID.canonical.y / tileScale,
             1.0 / tileScale / util::EXTENT,
             1.0 / tileScale / util::EXTENT}};
}

void MercatorProjection::tileMatrix(mat4& matrix, const UnwrappedTileID& tileID, double scale) const {
    mercatorTileMatrix(matrix, tileID, scale);
}

ProjectionData MercatorProjection::getProjectionData(const TransformState&,
                                                     const UnwrappedTileID& tileID,
                                                     const mat4& mercatorMatrix) const {
    return {.mainMatrix = mercatorMatrix,
            .tileMercatorCoords = mercatorTileCoords(tileID),
            .clippingPlane = {{0, 0, 0, 0}},
            .projectionTransition = 0,
            .fallbackMatrix = mercatorMatrix};
}

ProjectedTilePoint MercatorProjection::projectTilePoint(const ProjectionData& data,
                                                        const UnwrappedTileID&,
                                                        const Point<double>& point) const {
    vec4 pos = {{point.x, point.y, 0, 1}};
    matrix::transformMat4(pos, pos, data.mainMatrix);
    return {.point = {pos[0] / pos[3], pos[1] / pos[3]}, .signedDistanceFromCamera = pos[3], .occluded = false};
}

} // namespace mln
