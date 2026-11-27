#include <mln/map/mercator_projection.hpp>
#include <mln/util/projection.hpp>

#include <array>

namespace mln {

MercatorProjection::MercatorProjection(std::shared_ptr<const WorldCoordinateHelper> helper_)
    : helper(std::move(helper_)) {}

Point<double> MercatorProjection::project(const LatLng& latLng, double scale) const {
    if (isMercator(*helper)) {
        return Projection::project(latLng, scale);
    }
    return helper->worldFromLatLng(latLng) * Projection::worldSize(scale);
}

std::vector<Point<double>> MercatorProjection::project(const std::vector<LatLng>& latLngs, double scale) const {
    std::vector<std::array<double, 2>> lngLats;
    lngLats.reserve(latLngs.size());
    for (const LatLng& latLng : latLngs) {
        lngLats.push_back({latLng.longitude(), latLng.latitude()});
    }
    std::vector<std::array<double, 2>> world(lngLats.size());
    helper->worldFromLngLats(lngLats, world);
    std::vector<Point<double>> projected;
    projected.reserve(world.size());
    for (const auto& position : world) {
        projected.push_back(Point<double>{position[0], position[1]} * Projection::worldSize(scale));
    }
    return projected;
}

LatLng MercatorProjection::unproject(const Point<double>& point, double scale, LatLng::WrapMode wrapMode) const {
    if (isMercator(*helper)) {
        return Projection::unproject(point, scale, wrapMode);
    }
    return helper->latLngFromWorld(point / Projection::worldSize(scale));
}

} // namespace mln
