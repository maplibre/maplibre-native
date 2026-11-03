#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

using namespace mln;

namespace {

constexpr uint32_t demTileSize = 64;
// DEMData pads the tile with a 2px border on every side, and the texture is the padded image.
constexpr std::size_t demUploadBytes = (demTileSize + 4) * (demTileSize + 4) * 4;

// A Terrain-RGB tile with a gentle ramp, so the color-relief ramp has something to map.
std::string makeDEMTile() {
    PremultipliedImage image({demTileSize, demTileSize});
    for (uint32_t y = 0; y < demTileSize; ++y) {
        for (uint32_t x = 0; x < demTileSize; ++x) {
            uint8_t* px = image.data.get() + (y * demTileSize + x) * 4;
            px[0] = 1;
            px[1] = static_cast<uint8_t>(x * 4);
            px[2] = static_cast<uint8_t>(y * 4);
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

const char* colorReliefStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 0,
      "tileSize": 64
    }
  },
  "layers": [{
    "id": "relief",
    "type": "color-relief",
    "source": "dem",
    "paint": {
      "color-relief-color": [
        "interpolate", ["linear"], ["elevation"],
        -10000, "rgb(0, 0, 255)",
        0, "rgb(0, 255, 0)",
        10000, "rgb(255, 0, 0)"
      ]
    }
  }]
})STYLE";

} // namespace

// The DEM image backs the color-relief draw directly, with no prepare pass, and its pixels only
// change when the tile (re)loads or a neighbour backfills the border. It used to be re-uploaded
// for every visible tile on every frame - a full RGBA re-send of pixels that had not changed.
TEST(ColorRelief, UploadsDEMTextureOncePerTile) {
    util::RunLoop loop;

    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string tile = makeDEMTile();
    fileSource->tileResponse = [&](const Resource&) {
        Response res;
        res.data = std::make_shared<std::string>(tile);
        return res;
    };

    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(colorReliefStyle);
    map.jumpTo(CameraOptions().withCenter(LatLng{0, 0}).withZoom(0.0));

    std::vector<std::size_t> uploaded;
    std::size_t previous = 0;
    for (int frame = 0; frame < 4; ++frame) {
        const auto result = frontend.render(map);
        uploaded.push_back(result.stats.textureUpdateBytes - previous);
        previous = result.stats.textureUpdateBytes;
    }

    // Guard against the test passing because the layer never drew: some frame must have paid
    // for the DEM image at least once.
    EXPECT_TRUE(std::any_of(uploaded.begin(), uploaded.end(), [](std::size_t bytes) {
        return bytes >= demUploadBytes;
    })) << "the color-relief layer never uploaded a DEM texture; the test proves nothing";

    // Once the tile is up and nothing has changed, later frames must not re-send it.
    EXPECT_LT(uploaded.back(), demUploadBytes);
}
