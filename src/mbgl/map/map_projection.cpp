#include <mbgl/map/map_projection.hpp>
#include <mbgl/map/map_impl.hpp>
#include <mbgl/map/transform.hpp>

namespace mbgl {

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
    return transform->getCameraOptions(nullopt);
}

void MapProjection::setVisibleCoordinates(const std::vector<LatLng>& latLngs,
                                          const EdgeInsets& padding) {
    transform->jumpTo(mbgl::cameraForLatLngs(latLngs, *transform, padding)
                      .withBearing(-transform->getBearing() * util::RAD2DEG_D)
                      .withPitch(transform->getPitch() * util::RAD2DEG_D));
}

} // namespace mbgl
