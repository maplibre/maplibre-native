#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/map/map_options.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/io.hpp>
#include <mln/util/run_loop.hpp>

#include <algorithm>
#include <string>
#include <vector>

using namespace mln;
using namespace mln::style;
using namespace std::literals::string_literals;
using namespace std::chrono_literals;

TEST(Map, PrefetchTiles) {
    util::RunLoop runLoop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>();

    util::Timer emergencyShutoff;
    emergencyShutoff.start(10s, 0s, [&] {
        runLoop.stop();
        FAIL() << "Did not stop rendering";
    });

    StubMapObserver observer;
    observer.didFinishLoadingMapCallback = [&]() {
        runLoop.stop();
    };

    HeadlessFrontend frontend{{512, 512}, 1};
    MapAdapter map(
        frontend, observer, fileSource, MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));

    std::vector<int> tiles;

    fileSource->response = [&](const Resource& res) -> std::optional<Response> {
        static std::string tile = util::read_file("test/fixtures/map/prefetch/tile.png");

        auto zoom = std::stoi(res.url);
        tiles.push_back(zoom);

        Response response;
        response.data = std::make_shared<std::string>(tile);
        return response;
    };

    auto checkTilesForZoom = [&](int zoom, const std::vector<int>& expected) {
        tiles.clear();

        // Force tile reloading.
        map.getStyle().loadJSON(util::read_file("test/fixtures/map/prefetch/empty.json"));
        map.getStyle().loadJSON(util::read_file("test/fixtures/map/prefetch/style.json"));
        map.jumpTo(CameraOptions().withCenter(LatLng{40.726989, -73.992857}).withZoom(zoom)); // Manhattan
        runLoop.run();

        ASSERT_EQ(tiles.size(), expected.size());
        ASSERT_TRUE(std::is_permutation(tiles.begin(), tiles.end(), expected.begin()));
    };

    // Check defaults, should be 4.
    ASSERT_EQ(map.getPrefetchZoomDelta(), 4);
    checkTilesForZoom(12, {13, 13, 13, 13, 13, 13, 13, 13, 13, 9});

    // Setting it to 0 disables prefetching.
    map.setPrefetchZoomDelta(0);

    // No prefetching, raster tiles will use ideal
    // tiles instead of the actual zoom level, that is
    // why the zoom levels for non-prefetched tiles are
    // not the same.
    checkTilesForZoom(10, {11, 11, 11, 11, 11, 11, 11, 11, 11});

    map.setPrefetchZoomDelta(5);
    checkTilesForZoom(12, {13, 13, 13, 13, 13, 13, 13, 13, 13, 8});

    // Should clamp at `minzoom`.
    map.setPrefetchZoomDelta(20);
    checkTilesForZoom(10, {11, 11, 11, 11, 11, 11, 11, 11, 11, 0});

    // Disabled again.
    map.setPrefetchZoomDelta(0);
    checkTilesForZoom(13, {14, 14, 14, 14, 14, 14, 14, 14, 14});
}

namespace {

// Loads a view, moves to a distant view so that the first view's tiles leave
// the rendered set for the tile cache, then jumps back near the first view at a
// higher zoom. Renders one frame before any new tile has loaded and returns the
// color at the center of the view.
std::array<uint8_t, 4> centerPixelAfterJump(const CachedTileFallbackOptions& fallbackOptions,
                                            std::optional<bool> sourceOverride = std::nullopt) {
    util::RunLoop runLoop;
    auto fileSource = std::make_shared<StubFileSource>();

    util::Timer emergencyShutoff;
    emergencyShutoff.start(10s, 0s, [&] {
        runLoop.stop();
        ADD_FAILURE() << "Did not finish loading";
    });

    StubMapObserver observer;
    observer.didBecomeIdleCallback = [&]() {
        runLoop.stop();
    };

    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map(
        frontend, observer, fileSource, MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.setPrefetchZoomDelta(0);
    map.setCachedTileFallbackOptions(fallbackOptions);

    bool serveTiles = true;
    fileSource->response = [&](const Resource&) -> std::optional<Response> {
        static std::string tile = util::read_file("test/fixtures/map/prefetch/tile.png");
        if (!serveTiles) {
            return std::nullopt; // Stays pending.
        }
        Response response;
        response.data = std::make_shared<std::string>(tile);
        return response;
    };

    map.getStyle().loadJSON(R"STYLE({
      "version": 8,
      "sources": {
        "raster": { "type": "raster", "tiles": [ "{z}" ], "tileSize": 256, "maxzoom": 20 }
      },
      "layers": [
        { "id": "background", "type": "background", "paint": { "background-color": "blue" } },
        { "id": "raster", "type": "raster", "source": "raster", "paint": { "raster-fade-duration": 0 } }
      ]
    })STYLE");
    if (sourceOverride) {
        map.getStyle().getSource("raster")->setCachedTileFallbackEnabled(sourceOverride);
    }

    const LatLng center{40.726989, -73.992857};
    map.jumpTo(CameraOptions().withCenter(center).withZoom(2));
    runLoop.run();

    // Tiles of the first view are not needed here and go to the tile cache.
    map.jumpTo(CameraOptions().withCenter(LatLng{-40, 100}).withZoom(2));
    runLoop.run();

    // Jump to a zoom whose tiles were never loaded; they stay pending.
    serveTiles = false;
    map.jumpTo(CameraOptions().withCenter(center).withZoom(5));
    frontend.renderFrame();

    gfx::BackendScope scope{*frontend.getBackend()};
    const auto image = frontend.readStillImage();
    const size_t offset = 4 * ((image.size.height / 2) * image.size.width + image.size.width / 2);
    return {image.data[offset], image.data[offset + 1], image.data[offset + 2], image.data[offset + 3]};
}

constexpr std::array<uint8_t, 4> blue{0, 0, 255, 255};
constexpr std::array<uint8_t, 4> green{0, 255, 0, 255};

} // namespace

TEST(Map, CachedTileFallbackDefaults) {
    const CachedTileFallbackOptions defaults;
    EXPECT_FALSE(defaults.enabled);
    EXPECT_EQ(255, defaults.maxParentZoomDelta);
    EXPECT_EQ(2, defaults.maxChildZoomDelta);
}

TEST(Map, CachedTileFallbackDisabledShowsBackground) {
    EXPECT_EQ(blue, centerPixelAfterJump({}));
}

TEST(Map, CachedTileFallbackShowsCachedParent) {
    EXPECT_EQ(green, centerPixelAfterJump({.enabled = true}));
}

TEST(Map, CachedTileFallbackRespectsParentZoomDelta) {
    // The cached tiles are three levels up.
    EXPECT_EQ(blue, centerPixelAfterJump({.enabled = true, .maxParentZoomDelta = 2}));
    EXPECT_EQ(green, centerPixelAfterJump({.enabled = true, .maxParentZoomDelta = 3}));
}

TEST(Map, CachedTileFallbackSourceOverride) {
    EXPECT_EQ(green, centerPixelAfterJump({}, true));
    EXPECT_EQ(blue, centerPixelAfterJump({.enabled = true}, false));
}
