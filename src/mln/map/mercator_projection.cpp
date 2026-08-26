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

void MercatorProjection::tileMatrix(mat4& matrix, const UnwrappedTileID& tileID, double scale) const {
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

ProjectionData MercatorProjection::getProjectionData(const TransformState&,
                                                     const UnwrappedTileID& tileID,
                                                     const mat4& mercatorMatrix) const {
    const double tileScale = static_cast<double>(1ull << tileID.canonical.z);
    return {.mainMatrix = mercatorMatrix,
            .tileMercatorCoords = {{tileID.canonical.x / tileScale,
                                    tileID.canonical.y / tileScale,
                                    1.0 / tileScale / util::EXTENT,
                                    1.0 / tileScale / util::EXTENT}},
            .clippingPlane = {{0, 0, 0, 0}},
            .projectionTransition = 0,
            .fallbackMatrix = mercatorMatrix,
            .clipAntimeridian = false};
}

} // namespace mln
