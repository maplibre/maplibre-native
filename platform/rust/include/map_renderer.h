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

class MapRenderer {
public:
    MapRenderer() {}
    ~MapRenderer() {}

    void set_size(uint32_t width, uint32_t height) {
        size = {width, height};
    }

    void set_pixel_ratio(float ratio) {
        pixelRatio = ratio;
    }

    void set_map_mode(uint32_t mode) {
        mapMode = static_cast<mbgl::MapMode>(mode);
    }

    void set_debug_flags(uint32_t flags) {
        debugFlags = static_cast<mbgl::MapDebugOptions>(flags);
    }

    void set_camera(double lat, double lon, double zoom, double bearing, double pitch) {
        cameraOptions.withCenter(mbgl::LatLng{lat, lon})
                .withZoom(zoom)
                .withBearing(bearing)
                .withPitch(pitch);
    }

    void set_api_key(const std::string& key) {
        apiKey = key;
    }

    void set_cache_path(const std::string& path) {
        cachePath = path;
    }

    void set_asset_root(const std::string& path) {
        assetRoot = path;
    }

    void set_style_url(const std::string& url) {
        styleUrl = url;
    }

    // Main rendering method
    std::vector<uint8_t> render();

private:
    mbgl::util::RunLoop runLoop;
    mbgl::Size size {512, 512};
    float pixelRatio = 1.0f;
    mbgl::MapMode mapMode = mbgl::MapMode::Static;
    mbgl::MapDebugOptions debugFlags = mbgl::MapDebugOptions::NoDebug;
    mbgl::CameraOptions cameraOptions;
    std::string apiKey;
    std::string cachePath = "cache.sqlite";
    std::string assetRoot = ".";
    std::string styleUrl;
};
