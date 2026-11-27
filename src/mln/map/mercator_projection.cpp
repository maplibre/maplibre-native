#include <mln/map/mercator_projection.hpp>
#include <mln/util/projection.hpp>

namespace mln {

Point<double> MercatorProjection::project(const LatLng& latLng, double scale) const {
    return Projection::project(latLng, scale);
}

LatLng MercatorProjection::unproject(const Point<double>& point, double scale, LatLng::WrapMode wrapMode) const {
    return Projection::unproject(point, scale, wrapMode);
}

} // namespace mln
