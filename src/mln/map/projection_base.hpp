#pragma once

#include <mln/util/geo.hpp>
#include <mln/util/geometry.hpp>
#include <mln/util/mat4.hpp>

namespace mln {

class TransformState;
class UnwrappedTileID;

struct ProjectionData {
    mat4 mainMatrix{};
    vec4 tileMercatorCoords{};
    vec4 clippingPlane{};
    double projectionTransition = 0;
    mat4 fallbackMatrix{};
};

/// The tile-to-world matrix and the Mercator extent of a tile, the same on every projection.
void mercatorTileMatrix(mat4&, const UnwrappedTileID&, double scale);
vec4 mercatorTileCoords(const UnwrappedTileID&);

class ProjectionBase {
public:
    virtual ~ProjectionBase() = default;

    virtual Point<double> project(const LatLng&, double scale) const = 0;
    virtual LatLng unproject(const Point<double>&, double scale, LatLng::WrapMode) const = 0;

    virtual void tileMatrix(mat4&, const UnwrappedTileID&, double scale) const = 0;

    /// The per-tile projection contract, given the Mercator tile-to-clip matrix the renderer already computed.
    virtual ProjectionData getProjectionData(const TransformState&,
                                             const UnwrappedTileID&,
                                             const mat4& mercatorMatrix) const = 0;
};

} // namespace mln
