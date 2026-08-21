#include <mln/map/map_projection.hpp>
#include <mln/map/map_impl.hpp>
#include <mln/map/transform.hpp>
#include <mln/math/angles.hpp>

namespace mln {

MapProjection::MapProjection(const Map& map)
    : transform(std::make_unique<Transform>(map.getTransfromState())) {}

MapProjection::~MapProjection() = default;

ScreenCoordinate MapProjection::pixelForLatLng(const LatLng& latLng) const {
    // The implementation is just a copy from map.cpp
    LatLng unwrappedLatLng = latLng.wrapped();
    unwrappedLatLng.unwrapForShortestPath(transform->getLatLng());
    return transform->latLngToScreenCoordinate(unwrappedLatLng);
}

LatLng MapProjection::latLngForPixel(const ScreenCoordinate& pixel) const {
    // The implementation is just a copy from map.cpp
    return transform->screenCoordinateToLatLng(pixel);
}

void MapProjection::setCamera(const CameraOptions& camera) {
    transform->jumpTo(camera);
}

CameraOptions MapProjection::getCamera() const {
    return transform->getCameraOptions(std::nullopt);
}

void MapProjection::setVisibleCoordinates(const std::vector<LatLng>& latLngs, const EdgeInsets& padding) {
    transform->jumpTo(mln::cameraForLatLngs(latLngs, *transform, padding)
                          .withBearing(util::rad2deg(-transform->getBearing()))
                          .withPitch(util::rad2deg(transform->getPitch())));
}

} // namespace mln
