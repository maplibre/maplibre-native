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
    MapRenderer() {}
    ~MapRenderer() {}

    std::string render() {
        // Setup frontend and map
        HeadlessFrontend frontend(size, pixelRatio);

        auto tileServerOptions = TileServerOptions::MapTilerConfiguration();
        ResourceOptions resourceOptions;
        resourceOptions.withCachePath(cachePath).withAssetPath(assetRoot).withApiKey(apiKey).withTileServerOptions(
            tileServerOptions);

        Map map(frontend,
                MapObserver::nullObserver(),
                MapOptions().withMapMode(mapMode).withSize(size).withPixelRatio(pixelRatio),
                resourceOptions);

        // Handle style URL
        if (styleUrl.find("://") == std::string::npos) {
            styleUrl = "file://" + styleUrl;
        }

        // Load style and set camera
        map.getStyle().loadURL("https://demotiles.maplibre.org/style.json");
        map.jumpTo(cameraOptions);

        // Set debug flags
        if (debugFlags != MapDebugOptions::NoDebug) {
            map.setDebug(debugFlags);
        }

        // Render and encode
        auto image = frontend.render(map).image;
        return encodePNG(image);
    }

private:
    mbgl::util::RunLoop runLoop;

    // Due to CXX limitations, make all these public and access them with regular functions
public:
    mbgl::Size size{512, 512};
    float pixelRatio = 1.0f;
    mbgl::MapMode mapMode = mbgl::MapMode::Static;
    mbgl::MapDebugOptions debugFlags = mbgl::MapDebugOptions::NoDebug;
    mbgl::CameraOptions cameraOptions;
    std::string apiKey;
    std::string cachePath = "cache.sqlite";
    std::string assetRoot = ".";
    std::string styleUrl;
};

// MapRenderer constructor helpers
inline std::unique_ptr<MapRenderer> MapRenderer_new() {
    return std::make_unique<MapRenderer>();
}

inline std::unique_ptr<std::string> MapRenderer_render(MapRenderer& self) {
    return std::make_unique<std::string>(self.render());
}

inline void MapRenderer_setSize(MapRenderer& self, uint32_t width, uint32_t height) {
    self.size = {width, height};
}

inline void MapRenderer_setPixelRatio(MapRenderer& self, float ratio) {
    self.pixelRatio = ratio;
}

inline void MapRenderer_setMapMode(MapRenderer& self, mbgl::MapMode mode) {
    self.mapMode = mode;
}

inline void MapRenderer_setDebugFlags(MapRenderer& self, MapDebugOptions flags) {
    self.debugFlags = flags;
}

inline void MapRenderer_setCamera(
    MapRenderer& self, double lat, double lon, double zoom, double bearing, double pitch) {
    self.cameraOptions.withCenter(mbgl::LatLng{lat, lon}).withZoom(zoom).withBearing(bearing).withPitch(pitch);
}

inline void MapRenderer_setApiKey(MapRenderer& self, const rust::Str key) {
    self.apiKey = (std::string)key;
}

inline void MapRenderer_setCachePath(MapRenderer& self, const rust::Str path) {
    self.cachePath = (std::string)path;
}

inline void MapRenderer_setAssetRoot(MapRenderer& self, const rust::Str path) {
    self.assetRoot = (std::string)path;
}

inline void MapRenderer_setStyleUrl(MapRenderer& self, const rust::Str url) {
    self.styleUrl = (std::string)url;
}

} // namespace bridge
} // namespace mln
