#include <mbgl/map/camera.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/geo.hpp>

#include <memory>

namespace mbgl {

class Transform;

class MapProjection : private util::noncopyable {
public:
    explicit MapProjection(const Map&);
    ~MapProjection();

    ScreenCoordinate pixelForLatLng(const LatLng&) const;
    LatLng latLngForPixel(const ScreenCoordinate&) const;

    void setCamera(const CameraOptions&);
    CameraOptions getCamera() const;

    /// Set the underneath camera so the requested coordinates are visible with the inset.
    void setVisibleCoordinates(const std::vector<LatLng>&, const EdgeInsets&);

private:
    std::unique_ptr<Transform> transform;
};

} // namespace mbgl
