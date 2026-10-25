#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <string>

using namespace mln;

namespace {

// Past the DEM source's maxzoom, the terrain mesh and its drape targets keep following the view, so
// a low-maxzoom DEM does not soften the draped map. Flat ground from a DEM that stops at z11, under a
// raster of 2 px black and white stripes, seen straight down at zoom 14: the stripes must still be
// black and white on screen. With the drape held at the DEM's z11, each 1024 px target covered a z11
// tile, 2048 px of the view, and the stripes rendered as flat grey.
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
    "stripes": {
      "type": "raster",
      "tiles": ["http://example.com/stripes/{z}-{x}-{y}.png"],
      "maxzoom": 22,
      "tileSize": 256
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "stripes", "type": "raster", "source": "stripes", "paint": {"raster-fade-duration": 0}}]
})STYLE";

std::string makeTile(bool dem) {
    PremultipliedImage image({256, 256});
    for (uint32_t y = 0; y < 256; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            uint8_t* px = image.data.get() + (y * 256 + x) * 4;
            if (dem) {
                // Sea level in Terrain-RGB: (0 + 10000) / 0.1 = 100000 = 1 * 65536 + 134 * 256 + 160
                px[0] = 1;
                px[1] = 134;
                px[2] = 160;
            } else {
                const uint8_t v = (x / 2) % 2 ? 255 : 0;
                px[0] = px[1] = px[2] = v;
            }
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

} // namespace

TEST(TerrainDrape, ResolutionFollowsTheViewPastTheDEMMaxzoom) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string demTile = makeTile(true);
    const std::string stripesTile = makeTile(false);
    fileSource->tileResponse = [&](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(resource.url.find("/dem/") != std::string::npos ? demTile
                                                                                                 : stripesTile);
        return res;
    };
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(drapeStyle);
    map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(14.0).withPitch(0.0));

    PremultipliedImage image;
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        image = frontend.render(map).image;
    }

    // In the middle 64 x 64 pixels, count the near-black and near-white ones: the stripes, when the
    // drape resolves them. Blurred into one another they are all mid grey.
    int dark = 0;
    int light = 0;
    for (uint32_t y = 96; y < 160; ++y) {
        for (uint32_t x = 96; x < 160; ++x) {
            const uint8_t r = image.data[(y * image.size.width + x) * 4];
            dark += r < 60 ? 1 : 0;
            light += r > 195 ? 1 : 0;
        }
    }
    EXPECT_GT(dark, 64 * 64 / 4) << "light " << light;
    EXPECT_GT(light, 64 * 64 / 4) << "dark " << dark;
}
