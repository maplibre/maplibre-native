#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/image.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <algorithm>
#include <chrono>
#include <limits>
#include <sstream>
#include <string>
#include <thread>

using namespace mln;

// With terrain on, the tile cover holds tiles coarser than the camera's. A layer's minzoom
// then also decides which tiles build it, as in gl-js.
TEST(LayerMinZoomTiles, TileCoarserThanTheMinZoomDoesNotBuildTheLayer) {
    EXPECT_FALSE(tileZoomReachesLayerMinZoom(10, 12.0f));
    EXPECT_FALSE(tileZoomReachesLayerMinZoom(11, 12.0f));
    EXPECT_TRUE(tileZoomReachesLayerMinZoom(12, 12.0f));
    EXPECT_TRUE(tileZoomReachesLayerMinZoom(14, 12.0f));
}

TEST(LayerMinZoomTiles, FractionalMinZoomRoundsDownLikeGlJs) {
    EXPECT_TRUE(tileZoomReachesLayerMinZoom(12, 12.5f));
    EXPECT_FALSE(tileZoomReachesLayerMinZoom(11, 12.5f));
}

TEST(LayerMinZoomTiles, NoMinZoomKeepsEveryTile) {
    EXPECT_TRUE(tileZoomReachesLayerMinZoom(0, 0.0f));
    EXPECT_TRUE(tileZoomReachesLayerMinZoom(0, -std::numeric_limits<float>::infinity()));
}

namespace {

std::string makeFlatDEMTile() {
    constexpr uint32_t size = 64;
    PremultipliedImage image({size, size});
    for (uint32_t i = 0; i < size * size; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = 1;
        px[1] = 173;
        px[2] = 176;
        px[3] = 255;
    }
    return encodePNG(image);
}

// Points on a coarse grid across the view, drawn as icons from a geojson source, over terrain.
std::string pointsOverTerrainStyle() {
    std::ostringstream points;
    bool first = true;
    for (double lat = 36.0; lat < 40.5; lat += 0.2) {
        for (double lon = -124.0; lon < -117.5; lon += 0.2) {
            points << (first ? "" : ",") << "[" << lon << "," << lat << "]";
            first = false;
        }
    }
    return R"({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 12,
      "tileSize": 64
    },
    "points": {
      "type": "geojson",
      "data": {"type": "Feature", "properties": {}, "geometry": {"type": "MultiPoint", "coordinates": [)" +
           points.str() + R"(]}}
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{
    "id": "points",
    "type": "symbol",
    "source": "points",
    "layout": {"icon-image": "dot", "icon-allow-overlap": true, "icon-ignore-placement": true}
  }]
})";
}

} // namespace

// A layer's minzoom changed while the map runs does not rebuild the tiles that already hold the
// layer, so the coarse far-field tiles keep drawing it. The tiles are dropped at render time.
TEST(LayerMinZoomTiles, MinZoomChangedAtRuntimeDropsTheCoarseTilesUnderTerrain) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string tile = makeFlatDEMTile();
    fileSource->tileResponse = [&](const Resource&) {
        Response res;
        res.data = std::make_shared<std::string>(tile);
        return res;
    };
    HeadlessFrontend frontend{{430, 820}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(pointsOverTerrainStyle());
    PremultipliedImage icon({16, 16});
    std::fill_n(icon.data.get(), 16 * 16 * 4, uint8_t{255});
    map.getStyle().addImage(std::make_unique<style::Image>("dot", std::move(icon), 1.0f));
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.jumpTo(CameraOptions().withCenter(LatLng{38.26, -120.68}).withZoom(12.5).withPitch(80.0));

    const auto drawCallsAfter = [&](int frames) {
        int drawCalls = 0;
        for (int i = 0; i < frames; ++i) {
            loop.runOnce();
            drawCalls = frontend.render(map).stats.numDrawCalls;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        return drawCalls;
    };

    const int before = drawCallsAfter(20);
    ASSERT_GT(before, 0);

    map.getStyle().getLayer("points")->setMinZoom(12.0f);
    const int after = drawCallsAfter(20);
    EXPECT_LT(after, before);
}
