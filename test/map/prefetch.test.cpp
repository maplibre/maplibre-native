#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/map_adapter.hpp>

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
