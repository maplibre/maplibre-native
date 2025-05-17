#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>

#include <mbgl/gfx/backend.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/style/style.hpp>

#include <args.hxx>

#include <cstdlib>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    args::ArgumentParser argumentParser("MapLibre Native render tool");
    args::HelpFlag helpFlag(argumentParser, "help", "Display this help menu", {"help"});

    args::ValueFlag<std::string> backendValue(argumentParser, "Backend", "Rendering backend", {"backend"});
    args::ValueFlag<std::string> apikeyValue(argumentParser, "key", "API key", {'t', "apikey"});
    args::ValueFlag<std::string> styleValue(argumentParser, "URL", "Map stylesheet", {'s', "style"});
    args::ValueFlag<std::string> outputValue(argumentParser, "file", "Output file name", {'o', "output"});
    args::ValueFlag<std::string> cacheValue(argumentParser, "file", "Cache database file name", {'c', "cache"});
    args::ValueFlag<std::string> assetsValue(
        argumentParser, "file", "Directory to which asset:// URLs will resolve", {'a', "assets"});

    args::Flag debugFlag(argumentParser, "debug", "Debug mode", {"debug"});

    args::ValueFlag<double> pixelRatioValue(argumentParser, "number", "Image scale factor", {'r', "ratio"});

    // grouping ensures either bounds or center based position is used
    args::Group boundsOrCenterZoom(argumentParser, "Position (either one):", args::Group::Validators::AtMostOne);

    args::Group latLngBoundsGroup(boundsOrCenterZoom, "LatLng bounds:", args::Group::Validators::All);
    args::ValueFlag<double> northValue(latLngBoundsGroup, "degrees", "North latitude", {"north"});
    args::ValueFlag<double> westValue(latLngBoundsGroup, "degrees", "West longitude", {"west"});
    args::ValueFlag<double> southValue(latLngBoundsGroup, "degrees", "South latitude", {"south"});
    args::ValueFlag<double> eastValue(latLngBoundsGroup, "degrees", "East longitude", {"east"});

    args::Group centerGroup(boundsOrCenterZoom, "Center:", args::Group::Validators::AtLeastOne);
    args::ValueFlag<double> zoomValue(centerGroup, "number", "Zoom level", {'z', "zoom"});
    args::ValueFlag<double> lonValue(centerGroup, "degrees", "Longitude", {'x', "lon"});
    args::ValueFlag<double> latValue(centerGroup, "degrees", "Latitude", {'y', "lat"});

    args::ValueFlag<double> bearingValue(argumentParser, "degrees", "Bearing", {'b', "bearing"});
    args::ValueFlag<double> pitchValue(argumentParser, "degrees", "Pitch", {'p', "pitch"});
    args::ValueFlag<uint32_t> widthValue(argumentParser, "pixels", "Image width", {'w', "width"});
    args::ValueFlag<uint32_t> heightValue(argumentParser, "pixels", "Image height", {'h', "height"});

    args::ValueFlag<std::string> mapModeValue(
        argumentParser, "MapMode", "Map mode (e.g. 'static', 'tile', 'continuous')", {'m', "mode"});

    try {
        argumentParser.ParseCLI(argc, argv);
    } catch (const args::Help&) {
        std::cout << argumentParser;
        exit(0);
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << argumentParser;
        exit(1);
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << argumentParser;
        exit(2);
    }

    const double lat = latValue ? args::get(latValue) : 0;
    const double lon = lonValue ? args::get(lonValue) : 0;
    const double zoom = zoomValue ? args::get(zoomValue) : 0;
    const double bearing = bearingValue ? args::get(bearingValue) : 0;
    const double pitch = pitchValue ? args::get(pitchValue) : 0;
    const double pixelRatio = pixelRatioValue ? args::get(pixelRatioValue) : 1;

    const uint32_t width = widthValue ? args::get(widthValue) : 512;
    const uint32_t height = heightValue ? args::get(heightValue) : 512;
    const std::string output = outputValue ? args::get(outputValue) : "out.png";
    const std::string cache_file = cacheValue ? args::get(cacheValue) : "cache.sqlite";
    const std::string asset_root = assetsValue ? args::get(assetsValue) : ".";

    // Try to load the apikey from the environment.
    const char* apikeyEnv = getenv("MLN_API_KEY");
    const std::string apikey = apikeyValue ? args::get(apikeyValue) : (apikeyEnv ? apikeyEnv : std::string());

    const bool debug = debugFlag ? args::get(debugFlag) : false;

    using namespace mbgl;

    auto mapTilerConfiguration = mbgl::TileServerOptions::MapTilerConfiguration();
    std::string style = styleValue ? args::get(styleValue) : mapTilerConfiguration.defaultStyles().at(0).getUrl();

    util::RunLoop loop;

    MapMode mapMode = MapMode::Static;
    if (mapModeValue) {
        const auto modeStr = args::get(mapModeValue);
        if (modeStr == "tile") {
            mapMode = MapMode::Tile;
        } else if (modeStr == "continuous") {
            mapMode = MapMode::Continuous;
        }
    }

    HeadlessFrontend frontend({width, height}, static_cast<float>(pixelRatio));
    Map map(
        frontend,
        MapObserver::nullObserver(),
        MapOptions().withMapMode(mapMode).withSize(frontend.getSize()).withPixelRatio(static_cast<float>(pixelRatio)),
        ResourceOptions()
            .withCachePath(cache_file)
            .withAssetPath(asset_root)
            .withApiKey(apikey)
            .withTileServerOptions(mapTilerConfiguration));

    if (style.find("://") == std::string::npos) {
        style = std::string("file://") + style;
    }

    map.getStyle().loadURL(style);
    if (latLngBoundsGroup.Get()) {
        LatLngBounds boundingBox = LatLngBounds::hull(LatLng(args::get(northValue), args::get(westValue)),
                                                      LatLng(args::get(southValue), args::get(eastValue)));
        map.jumpTo(map.cameraForLatLngBounds(boundingBox, EdgeInsets(), bearing, pitch));
    } else {
        map.jumpTo(CameraOptions().withCenter(LatLng{lat, lon}).withZoom(zoom).withBearing(bearing).withPitch(pitch));
    }

    if (debug) {
        map.setDebug(debug ? mbgl::MapDebugOptions::TileBorders | mbgl::MapDebugOptions::ParseStatus
                           : mbgl::MapDebugOptions::NoDebug);
    }

    try {
        std::ofstream out(output, std::ios::binary);
        out << encodePNG(frontend.render(map).image);
        out.close();
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
