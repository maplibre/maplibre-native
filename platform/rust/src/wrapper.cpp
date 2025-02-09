#include "maplibre-native/include/tile_server_options.h"
#include "maplibre-native/include/map_renderer.h"
#include "maplibre-native/src/lib.rs.h"

namespace ml {
namespace bridge {

std::string MapRenderer::render() {
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

} // namespace bridge
} // namespace ml
