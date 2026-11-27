#pragma once

#include <mln/map/world_coordinate_helper.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/geometry.hpp>

#include <vector>

namespace mln {

class ProjectionBase {
public:
    virtual ~ProjectionBase() = default;

    virtual const WorldCoordinateHelper& worldCoordinateHelper() const = 0;

    virtual Point<double> project(const LatLng&, double scale) const = 0;
    /// `project` for each lat/lng, converted in one batch in a registered projection.
    virtual std::vector<Point<double>> project(const std::vector<LatLng>&, double scale) const = 0;
    virtual LatLng unproject(const Point<double>&, double scale, LatLng::WrapMode) const = 0;
};

} // namespace mln
