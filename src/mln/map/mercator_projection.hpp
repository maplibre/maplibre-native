#pragma once

#include <mln/map/projection_base.hpp>

namespace mln {

class MercatorProjection final : public ProjectionBase {
public:
    Point<double> project(const LatLng&, double scale) const override;
    LatLng unproject(const Point<double>&, double scale, LatLng::WrapMode) const override;

    void tileMatrix(mat4&, const UnwrappedTileID&, double scale) const override;

    ProjectionData getProjectionData(const UnwrappedTileID&, double scale, const mat4& projMatrix) const override;
    ProjectionData getProjectionData(const UnwrappedTileID&, const mat4& mainMatrix) const override;
};

} // namespace mln
