#pragma once

#include <mln/util/geo.hpp>
#include <mln/util/geometry.hpp>

namespace mln {

class ProjectionBase {
public:
    virtual ~ProjectionBase() = default;

    virtual Point<double> project(const LatLng&, double scale) const = 0;
    virtual LatLng unproject(const Point<double>&, double scale, LatLng::WrapMode) const = 0;
};

} // namespace mln
