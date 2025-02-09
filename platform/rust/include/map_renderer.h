#pragma once

#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/style/style.hpp>
#include <memory>
#include <vector>
#include <stdexcept>

namespace mln {
namespace bridge {

class MapRenderer {
public:
    explicit MapRenderer(std::unique_ptr<mbgl::HeadlessFrontend> frontendInstance,
                         std::unique_ptr<mbgl::Map> mapInstance)
        : frontend(std::move(frontendInstance)),
          map(std::move(mapInstance)) {}
    ~MapRenderer() {}

public:
    mbgl::util::RunLoop runLoop;
    // Due to CXX limitations, make all these public and access them from the regular functions below
    std::unique_ptr<mbgl::HeadlessFrontend> frontend;
    std::unique_ptr<mbgl::Map> map;
};

inline std::unique_ptr<MapRenderer> MapRenderer_new(mbgl::MapMode mapMode,
                                                    uint32_t width,
                                                    uint32_t height,
                                                    float pixelRatio,
                                                    const rust::Str cachePath,
                                                    const rust::Str assetRoot,
                                                    const rust::Str apiKey) {
    mbgl::Size size = {width, height};

    auto frontend = std::make_unique<mbgl::HeadlessFrontend>(size, pixelRatio);

    // TODO: fix this to use the correct tile server options
    auto tileServerOptions = TileServerOptions::MapTilerConfiguration();

    ResourceOptions resourceOptions;
    resourceOptions.withCachePath((std::string)cachePath)
        .withAssetPath((std::string)assetRoot)
        .withApiKey((std::string)apiKey)
        .withTileServerOptions(tileServerOptions);

    MapOptions mapOptions;
    mapOptions.withMapMode(mapMode).withSize(size).withPixelRatio(pixelRatio);

    auto map = std::make_unique<mbgl::Map>(*frontend, MapObserver::nullObserver(), mapOptions, resourceOptions);

    return std::make_unique<MapRenderer>(std::move(frontend), std::move(map));
}

inline std::unique_ptr<std::string> MapRenderer_render(MapRenderer& self) {
    auto image = encodePNG(self.frontend->render(*self.map).image);
    return std::make_unique<std::string>(image);
}

inline void MapRenderer_setDebugFlags(MapRenderer& self, mbgl::MapDebugOptions debugFlags) {
    self.map->setDebug(debugFlags);
}

inline void MapRenderer_setCamera(
    MapRenderer& self, double lat, double lon, double zoom, double bearing, double pitch) {
    // TODO: decide if this is the right approach,
    //       or if we want to cache camera options in the instance,
    //       and have several setters for each property.
    mbgl::CameraOptions cameraOptions;
    cameraOptions.withCenter(mbgl::LatLng{lat, lon}).withZoom(zoom).withBearing(bearing).withPitch(pitch);
    self.map->jumpTo(cameraOptions);
}

inline void MapRenderer_setStyleUrl(MapRenderer& self, const rust::Str styleUrl) {
    self.map->getStyle().loadURL((std::string)styleUrl);
}

} // namespace bridge
} // namespace mln
