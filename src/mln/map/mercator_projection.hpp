#pragma once

#include <mln/map/projection_base.hpp>

namespace mln {

class MercatorProjection final : public ProjectionBase {
public:
    Point<double> project(const LatLng&, double scale) const override;
    LatLng unproject(const Point<double>&, double scale, LatLng::WrapMode) const override;
};

} // namespace mln
