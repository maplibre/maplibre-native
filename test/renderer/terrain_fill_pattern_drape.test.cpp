#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/image.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <memory>
#include <string>

using namespace mln;

namespace {

// Flat ground under a fill-pattern layer: a polygon that covers everything west of the map center,
// from a GeoJSON source that stops at z10, drawn straight down at zoom 16 where the drape targets
// are far deeper than that tile. A solid red image is the pattern, so a pixel is red exactly where
// the polygon is.
const char* drapeStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 11,
      "tileSize": 256
    },
    "west": {
      "type": "geojson",
      "maxzoom": 10,
      "data": {
        "type": "Feature",
        "properties": {},
        "geometry": {
          "type": "Polygon",
          "coordinates": [[[9.98, 45.98], [10.0, 45.98], [10.0, 46.02], [9.98, 46.02], [9.98, 45.98]]]
        }
      }
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "west", "type": "fill", "source": "west",
              "paint": {"fill-pattern": "red", "fill-antialias": false}}]
})STYLE";

std::string demTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t i = 0; i < 256 * 256; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        // Sea level in Terrain-RGB: (0 + 10000) / 0.1 = 100000 = 1 * 65536 + 134 * 256 + 160
        px[0] = 1;
        px[1] = 134;
        px[2] = 160;
        px[3] = 255;
    }
    return encodePNG(image);
}

std::unique_ptr<style::Image> redPattern() {
    PremultipliedImage image({8, 8});
    for (uint32_t i = 0; i < 8 * 8; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = 255;
        px[1] = 0;
        px[2] = 0;
        px[3] = 255;
    }
    return std::make_unique<style::Image>("red", std::move(image), 1.0f);
}

} // namespace

// A fill-pattern drawable is placed into each drape target by the vertex shader from its own tile
// and the target's. The Metal pattern vertex stage did not place it: the whole tile was stretched
// over every target it touched, so the polygon showed as a thin strip near the middle instead of
// the west half of the view.
TEST(TerrainDrape, AFillPatternFromAShallowTileLandsWhereItsPolygonIs) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string dem = demTile();
    fileSource->tileResponse = [&](const Resource&) {
        Response res;
        res.data = std::make_shared<std::string>(dem);
        return res;
    };
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(drapeStyle);
    map.getStyle().addImage(redPattern());
    map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(16.0).withPitch(0.0));

    PremultipliedImage image;
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        image = frontend.render(map).image;
    }

    const auto red = [&](uint32_t x, uint32_t y) {
        const uint8_t* px = image.data.get() + (y * image.size.width + x) * 4;
        return px[0] > 150 && px[1] < 90 && px[2] < 90;
    };
    int west = 0;
    int east = 0;
    for (uint32_t y = 96; y < 160; ++y) {
        for (uint32_t x = 16; x < 100; ++x) west += red(x, y) ? 1 : 0;
        for (uint32_t x = 156; x < 240; ++x) east += red(x, y) ? 1 : 0;
    }
    const int area = 64 * 84;
    EXPECT_GT(west, area * 9 / 10) << "east " << east;
    EXPECT_LT(east, area / 20) << "west " << west;
}
