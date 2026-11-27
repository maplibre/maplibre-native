#pragma once

#include <mln/map/projection_base.hpp>

namespace mln {

class MercatorProjection final : public ProjectionBase {
public:
    explicit MercatorProjection(std::shared_ptr<const WorldCoordinateHelper> = mercatorWorldCoordinateHelper());

    const WorldCoordinateHelper& worldCoordinateHelper() const override { return *helper; }

    Point<double> project(const LatLng&, double scale) const override;
    std::vector<Point<double>> project(const std::vector<LatLng>&, double scale) const override;
    LatLng unproject(const Point<double>&, double scale, LatLng::WrapMode) const override;

private:
    std::shared_ptr<const WorldCoordinateHelper> helper;
};

} // namespace mln
