#include "glfw_view.hpp"
#include "glfw_renderer_frontend.hpp"
#include "settings_json.hpp"

#include <mbgl/gfx/backend.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/storage/database_file_source.hpp>
#include <mbgl/storage/file_source_manager.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/string.hpp>

#include <args.hxx>

#include <csignal>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <array>

namespace {

GLFWView* view = nullptr;

} // namespace

void quit_handler(int) {
    if (view) {
        mbgl::Log::Info(mbgl::Event::Setup, "waiting for quit...");
        view->setShouldClose();
    } else {
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    args::ArgumentParser argumentParser("Mapbox GL GLFW example");
    args::HelpFlag helpFlag(argumentParser, "help", "Display this help menu", {'h', "help"});

    args::Flag fullscreenFlag(argumentParser, "fullscreen", "Toggle fullscreen", {'f', "fullscreen"});
    args::Flag benchmarkFlag(argumentParser, "benchmark", "Toggle benchmark", {'b', "benchmark"});
    args::Flag offlineFlag(argumentParser, "offline", "Toggle offline", {'o', "offline"});

    args::ValueFlag<std::string> testDirValue(
        argumentParser, "directory", "Root directory for test generation", {"testDir"});
    args::ValueFlag<std::string> backendValue(argumentParser, "backend", "Rendering backend", {"backend"});
    args::ValueFlag<std::string> apikeyValue(argumentParser, "key", "API key", {'t', "apikey"});
    args::ValueFlag<std::string> styleValue(argumentParser, "URL", "Map stylesheet", {'s', "style"});
    args::ValueFlag<std::string> cacheDBValue(argumentParser, "file", "Cache database file name", {'c', "cache"});
    args::ValueFlag<double> lonValue(argumentParser, "degrees", "Longitude", {'x', "lon"});
    args::ValueFlag<double> latValue(argumentParser, "degrees", "Latitude", {'y', "lat"});
    args::ValueFlag<double> zoomValue(argumentParser, "number", "Zoom level", {'z', "zoom"});
    args::ValueFlag<double> bearingValue(argumentParser, "degrees", "Bearing", {'b', "bearing"});
    args::ValueFlag<double> pitchValue(argumentParser, "degrees", "Pitch", {'p', "pitch"});

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

    // Load settings
    mbgl::Settings_JSON settings;
    settings.online = !offlineFlag;
    if (lonValue) settings.longitude = args::get(lonValue);
    if (latValue) settings.latitude = args::get(latValue);
    if (zoomValue) settings.zoom = args::get(zoomValue);
    if (bearingValue) settings.bearing = args::get(bearingValue);
    if (pitchValue) settings.pitch = args::get(pitchValue);

    const bool fullscreen = fullscreenFlag ? args::get(fullscreenFlag) : false;
    const bool benchmark = benchmarkFlag ? args::get(benchmarkFlag) : false;
    std::string style = styleValue ? args::get(styleValue) : "";
    const std::string cacheDB = cacheDBValue ? args::get(cacheDBValue) : "/tmp/mbgl-cache.db";

    // sigint handling
#ifdef WIN32
    signal(SIGINT, &quit_handler);
#else
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = quit_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
#endif

    if (benchmark) {
        mbgl::Log::Info(mbgl::Event::General, "BENCHMARK MODE: Some optimizations are disabled.");
    }

    // Set access token if present
    const char* apikeyEnv = getenv("MLN_API_KEY");
    const std::string apikey = apikeyValue ? args::get(apikeyValue) : (apikeyEnv ? apikeyEnv : std::string());

    auto mapTilerConfiguration = mbgl::TileServerOptions::MapTilerConfiguration();
    mbgl::ResourceOptions resourceOptions;
    resourceOptions.withCachePath(cacheDB).withApiKey(apikey).withTileServerOptions(mapTilerConfiguration);
    mbgl::ClientOptions clientOptions;
    auto orderedStyles = mapTilerConfiguration.defaultStyles();

    GLFWView backend(fullscreen, benchmark, resourceOptions, clientOptions);
    view = &backend;

    std::shared_ptr<mbgl::FileSource> onlineFileSource = mbgl::FileSourceManager::get()->getFileSource(
        mbgl::FileSourceType::Network, resourceOptions, clientOptions);
    if (!settings.online) {
        if (onlineFileSource) {
            onlineFileSource->setProperty("online-status", false);
            mbgl::Log::Warning(mbgl::Event::Setup, "Application is offline. Press `O` to toggle online status.");
        } else {
            mbgl::Log::Warning(mbgl::Event::Setup,
                               "Network resource provider is not available, only local "
                               "requests are supported.");
        }
    }

    GLFWRendererFrontend rendererFrontend{
        std::make_unique<mbgl::Renderer>(view->getRendererBackend(), view->getPixelRatio()), *view};

    mbgl::Map map(rendererFrontend,
                  *view,
                  mbgl::MapOptions().withSize(view->getSize()).withPixelRatio(view->getPixelRatio()),
                  resourceOptions,
                  clientOptions);

    backend.setMap(&map);

    if (!style.empty() && style.find("://") == std::string::npos) {
        style = std::string("file://") + style;
    }

    map.jumpTo(mbgl::CameraOptions()
                   .withCenter(mbgl::LatLng{settings.latitude, settings.longitude})
                   .withZoom(settings.zoom)
                   .withBearing(settings.bearing)
                   .withPitch(settings.pitch));
    map.setDebug(mbgl::MapDebugOptions(settings.debug));

    if (testDirValue) view->setTestDirectory(args::get(testDirValue));

    view->setOnlineStatusCallback([&settings, onlineFileSource]() {
        if (!onlineFileSource) {
            mbgl::Log::Warning(mbgl::Event::Setup,
                               "Cannot change online status. Network resource provider is not "
                               "available.");
            return;
        }
        settings.online = !settings.online;
        onlineFileSource->setProperty("online-status", settings.online);
        mbgl::Log::Info(mbgl::Event::Setup,
                        std::string("Application is ") + (settings.online ? "online" : "offline") +
                            ". Press `O` to toggle online status.");
    });

    view->setChangeStyleCallback([&map, &orderedStyles]() {
        static uint8_t currentStyleIndex;

        if (++currentStyleIndex == orderedStyles.size()) {
            currentStyleIndex = 0;
        }

        mbgl::util::DefaultStyle newStyle = orderedStyles[currentStyleIndex];
        map.getStyle().loadURL(newStyle.getUrl());
        view->setWindowTitle(newStyle.getName());

        mbgl::Log::Info(mbgl::Event::Setup, "Changed style to: " + newStyle.getName());
    });

    // Resource loader controls top-level request processing and can resume /
    // pause all managed sources simultaneously.
    std::shared_ptr<mbgl::FileSource> resourceLoader = mbgl::FileSourceManager::get()->getFileSource(
        mbgl::FileSourceType::ResourceLoader, resourceOptions, clientOptions);
    view->setPauseResumeCallback([resourceLoader]() {
        static bool isPaused = false;

        if (isPaused) {
            resourceLoader->resume();
        } else {
            resourceLoader->pause();
        }

        isPaused = !isPaused;
    });

    // Database file source.
    auto databaseFileSource = std::static_pointer_cast<mbgl::DatabaseFileSource>(std::shared_ptr<mbgl::FileSource>(
        mbgl::FileSourceManager::get()->getFileSource(mbgl::FileSourceType::Database, resourceOptions, clientOptions)));
    view->setResetCacheCallback([databaseFileSource]() {
        databaseFileSource->resetDatabase([](const std::exception_ptr& ex) {
            if (ex) {
                mbgl::Log::Error(mbgl::Event::Database, "Failed to reset cache: " + mbgl::util::toString(ex));
            }
        });
    });

    // Load style
    if (style.empty()) {
        const char* url = getenv("MAPBOX_STYLE_URL");
        if (url == nullptr) {
            mbgl::util::DefaultStyle newStyle = orderedStyles[0];
            style = newStyle.getUrl();
            view->setWindowTitle(newStyle.getName());
        } else {
            style = url;
            view->setWindowTitle(url);
        }
    }

    map.getStyle().loadURL(style);

    view->run();

    // Save settings
    mbgl::CameraOptions camera = map.getCameraOptions();
    settings.latitude = camera.center->latitude();
    settings.longitude = camera.center->longitude();
    settings.zoom = *camera.zoom;
    settings.bearing = *camera.bearing;
    settings.pitch = *camera.pitch;
    settings.debug = mbgl::EnumType(map.getDebug());
    settings.save();
    mbgl::Log::Info(mbgl::Event::General,
                    "Exit location: --lat=\"" + std::to_string(settings.latitude) + "\" --lon=\"" +
                        std::to_string(settings.longitude) + "\" --zoom=\"" + std::to_string(settings.zoom) +
                        "\" --bearing=\"" + std::to_string(settings.bearing) + "\"");

    view = nullptr;

    return 0;
}
