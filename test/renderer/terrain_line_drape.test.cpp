#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <memory>
#include <string>

using namespace mln;

namespace {

// A red line running north-south a little west of the map center, with a gradient and dashes (the
// line-gradient SDF shader), from a GeoJSON source that reaches the view's zoom. Flat ground at sea
// level under it when `terrain`, seen straight down at zoom 16.
std::string lineStyle(bool terrain) {
    const std::string terrainEntry = terrain ? R"("terrain": {"source": "dem", "exaggeration": 1.0},)" : "";
    return R"({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 11,
      "tileSize": 256
    },
    "line": {
      "type": "geojson",
      "maxzoom": 16,
      "lineMetrics": true,
      "data": {
        "type": "Feature",
        "properties": {},
        "geometry": {"type": "LineString", "coordinates": [[9.9995, 45.99], [9.9995, 46.01]]}
      }
    }
  },)" + terrainEntry +
           R"(
  "layers": [{"id": "line", "type": "line", "source": "line",
              "paint": {"line-width": 16, "line-dasharray": [2, 1],
                        "line-gradient": ["interpolate", ["linear"], ["line-progress"], 0, "red", 1, "red"]}}]
})";
}

std::string seaLevelDEMTile() {
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

struct RedPixels {
    int band = 0;    // in the band where the line is
    int outside = 0; // anywhere else
};

RedPixels renderLine(bool terrain) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string dem = seaLevelDEMTile();
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
    map.getStyle().loadJSON(lineStyle(terrain));
    map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(16.0).withPitch(0.0));

    PremultipliedImage image;
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        image = frontend.render(map).image;
    }

    // At zoom 16 the line, 0.0005° west of the center, lies about 47 px left of it (x 128).
    RedPixels red;
    for (uint32_t y = 16; y < 240; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            const uint8_t* px = image.data.get() + (y * image.size.width + x) * 4;
            if (px[0] > 150 && px[1] < 90 && px[2] < 90) {
                (x >= 70 && x < 92 ? red.band : red.outside) += 1;
            }
        }
    }
    return red;
}

} // namespace

// Each draped vertex shader places its tile into the drape target it is drawing with
// apply_drape_transform. The line-gradient SDF shader (a line with both line-gradient and
// line-dasharray) did not, on any backend: under terrain the whole tile was drawn into every target
// it touched as if it filled it, so the dashes landed away from the line.
TEST(TerrainDrape, ADashedGradientLineLandsWhereItIs) {
    const RedPixels flat = renderLine(false);
    ASSERT_GT(flat.band, 1000) << "the line draws without terrain";
    ASSERT_LT(flat.outside, 50);

    const RedPixels draped = renderLine(true);
    EXPECT_GT(draped.band, flat.band * 8 / 10) << "outside " << draped.outside;
    EXPECT_LT(draped.outside, flat.band / 20) << "band " << draped.band;
}
