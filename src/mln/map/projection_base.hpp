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
    /// Clip-space Z shift for this drawable's layer, the same one the Mercator matrix carries.
    double depthOffset = 0;
    /// The layer's translation in tile units, added on the sphere; the fallback matrix already carries it.
    vec2 translate{};
};

/// A tile point in clip space, as the vertex shaders would place it.
struct ProjectedTilePoint {
    Point<double> point;
    double signedDistanceFromCamera = 0;
    /// Behind the planet's horizon; never true on Mercator.
    bool occluded = false;
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

    /// The clip-space position of a tile point, computed the way the vertex shaders do it.
    virtual ProjectedTilePoint projectTilePoint(const ProjectionData&,
                                                const UnwrappedTileID&,
                                                const Point<double>&) const = 0;

    /// Scale that keeps map-aligned circles and pitched text the size they have on Mercator at the map center.
    virtual double circleRadiusCorrection(const TransformState&) const { return 1.0; }

    /// Ratio of a screen pixel at the map center to one at the same zoom on Mercator.
    virtual double pixelScale(const TransformState&) const { return 1.0; }

    /// `circleRadiusCorrection` extended to a point away from the center, by its latitude.
    virtual double pitchedTextCorrection(const TransformState&, const Point<double>&, const UnwrappedTileID&) const {
        return 1.0;
    }
};

} // namespace mln
