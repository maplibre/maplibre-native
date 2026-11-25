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

// Flat ground under a blue fill with a red outline, straight down at zoom 16. The outline shaders place
// the line in pixels of the target they draw into, so every drape target has to draw with its own
// global paint parameters. Any red in a pixel is the outline: the fill is pure blue.
const char* outlineStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 11,
      "tileSize": 256
    },
    "square": {
      "type": "geojson",
      "data": {
        "type": "Feature",
        "properties": {},
        "geometry": {
          "type": "Polygon",
          "coordinates": [[[9.9994, 45.9996], [10.0006, 45.9996], [10.0006, 46.0004], [9.9994, 46.0004], [9.9994, 45.9996]]]
        }
      }
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "square", "type": "fill", "source": "square",
              "paint": {"fill-color": "#0000ff", "fill-outline-color": "#ff0000"}}]
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

} // namespace

// On Vulkan the global paint parameters of a drape target never reached its pass: one descriptor set was
// rewritten for every swap while the earlier passes were only recorded, so each pass read the last block
// written, which carries the screen's size. The outline then never landed in the drape texture and the
// polygon showed no outline at all.
TEST(TerrainDrape, AFillOutlineIsDrawnOnTerrain) {
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
    map.getStyle().loadJSON(outlineStyle);
    map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(16.0).withPitch(0.0));

    PremultipliedImage image;
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        image = frontend.render(map).image;
    }

    int outline = 0;
    int fill = 0;
    for (uint32_t y = 0; y < image.size.height; ++y) {
        for (uint32_t x = 0; x < image.size.width; ++x) {
            const uint8_t* px = image.data.get() + (y * image.size.width + x) * 4;
            outline += px[0] > 8 ? 1 : 0;
            fill += (px[0] < 40 && px[1] < 40 && px[2] > 200) ? 1 : 0;
        }
    }
    // The polygon covers about 112 x 108 pixels; its outline is a faint line about a pixel wide around it (about 200
    // pixels with any red).
    EXPECT_GT(fill, 8000) << "the polygon is not drawn";
    EXPECT_LT(fill, 20000) << "the polygon fills the view";
    EXPECT_GT(outline, 100) << "fill pixels " << fill;
}
