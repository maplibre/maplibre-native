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

// A red line running north-south a little west of the map center, 16 px wide, from a GeoJSON source
// that stops at zoom 12, seen straight down at zoom 16: its tiles are four zooms shallower than the
// view, so under terrain they are drawn into drape targets much deeper than themselves. Flat ground
// at sea level under it when `terrain`.
std::string lineStyle(bool terrain, const std::string& paint) {
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
      "maxzoom": 12,
      "lineMetrics": true,
      "data": {
        "type": "Feature",
        "properties": {},
        "geometry": {"type": "LineString", "coordinates": [[9.9995, 45.99], [9.9995, 46.01]]}
      }
    }
  },)" + terrainEntry +
           R"(
  "layers": [{"id": "line", "type": "line", "source": "line", "paint": )" +
           paint + R"(}]
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

// Solid red pixels in the band where the line is (about 47 px west of the center).
int renderLine(bool terrain, const std::string& paint) {
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
    map.getStyle().loadJSON(lineStyle(terrain, paint));
    PremultipliedImage red({8, 8});
    for (uint32_t i = 0; i < 8 * 8; ++i) {
        uint8_t* px = red.data.get() + i * 4;
        px[0] = 255;
        px[1] = 0;
        px[2] = 0;
        px[3] = 255;
    }
    map.getStyle().addImage(std::make_unique<style::Image>("red", std::move(red), 1.0f));
    map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(16.0).withPitch(0.0));

    PremultipliedImage image;
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        image = frontend.render(map).image;
    }

    int solid = 0;
    for (uint32_t y = 16; y < 240; ++y) {
        for (uint32_t x = 70; x < 92; ++x) {
            const uint8_t* px = image.data.get() + (y * image.size.width + x) * 4;
            solid += px[0] > 150 && px[1] < 90 && px[2] < 90 ? 1 : 0;
        }
    }
    return solid;
}

} // namespace

// A line drawn into a drape target measures its antialiasing in drape pixels: the target is drawn
// with an orthographic matrix and the perspective comes when the terrain mesh is drawn. The plain
// line shader set its gamma scale to 1 there; the gradient, pattern and dash (SDF) shaders measured
// the extrude with the tile's own matrix instead. For a tile shallower than the target that matrix
// is the target's scaled down by the zoom difference, so their edges blurred by that factor and the
// lines came out faint, more so the shallower their tiles.
TEST(TerrainDrape, LinesFromShallowerTilesKeepTheirEdges) {
    const struct {
        const char* name;
        const char* paint;
    } kinds[] = {
        {"plain", R"({"line-width": 16, "line-color": "red"})"},
        {"gradient",
         R"({"line-width": 16, "line-gradient": ["interpolate", ["linear"], ["line-progress"], 0, "red", 1, "red"]})"},
        {"dashed", R"({"line-width": 16, "line-color": "red", "line-dasharray": [2, 1]})"},
        {"pattern", R"({"line-width": 16, "line-pattern": "red"})"},
    };
    for (const auto& kind : kinds) {
        const int flat = renderLine(false, kind.paint);
        ASSERT_GT(flat, 1000) << kind.name << " draws without terrain";
        const int draped = renderLine(true, kind.paint);
        EXPECT_GT(draped, flat * 8 / 10) << kind.name << ": " << draped << " solid red pixels draped, " << flat
                                         << " without terrain";
    }
}
