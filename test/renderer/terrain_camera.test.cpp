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
// Terrain-RGB encodes elevation as -10000 + (R * 65536 + G * 256 + B) * 0.1, so a flat
// 1000 m plateau is RGB(1, 173, 176): 65536 + 44288 + 176 = 110000.
constexpr double plateauMeters = 1000.0;

std::string makeFlatDEMTile() {
    PremultipliedImage image({demTileSize, demTileSize});
    for (uint32_t i = 0; i < demTileSize * demTileSize; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = 1;
        px[1] = 173;
        px[2] = 176;
        px[3] = 255;
    }
    return encodePNG(image);
}

const char* terrainStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 2,
      "tileSize": 64
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": []
})STYLE";

struct TerrainCameraTest {
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    std::string tile = makeFlatDEMTile();
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map;

    TerrainCameraTest()
        : map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())) {
        fileSource->tileResponse = [this](const Resource&) {
            Response res;
            res.data = std::make_shared<std::string>(tile);
            return res;
        };
        map.getStyle().loadJSON(terrainStyle);
        map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(2.0).withPitch(60.0));
    }

    double renderAndGetAltitude(int frames) {
        double altitude = 0.0;
        for (int i = 0; i < frames; ++i) {
            loop.runOnce();
            frontend.render(map);
            altitude = map.getCameraOptions({}).centerAltitude.value_or(0.0);
        }
        return altitude;
    }
};

} // namespace

// The centre rides the terrain instead of sea level, so pitching over high ground does not
// leave the camera inside the hillside.
TEST(TerrainCamera, CentreRidesTheTerrainSurface) {
    TerrainCameraTest test;
    EXPECT_TRUE(test.map.getCenterClampedToGround());

    const double settled = test.renderAndGetAltitude(8);
    EXPECT_NEAR(settled, plateauMeters, 5.0);
}

// The historical worry about a terrain-anchored centre was that raising it re-samples a new
// elevation and runs away. Raising the centre moves the orbit plane, not the centre's lng/lat,
// so it must settle - and the centre must not drift while it does.
TEST(TerrainCamera, CentreElevationSettlesWithoutDrift) {
    TerrainCameraTest test;

    test.renderAndGetAltitude(8);
    const auto afterSettling = test.map.getCameraOptions({});
    const double settledAltitude = *afterSettling.centerAltitude;
    // Guard against settling "stably" at sea level, which would make the rest vacuous.
    ASSERT_NEAR(settledAltitude, plateauMeters, 5.0);

    // Ten more frames with nothing else changing.
    const double later = test.renderAndGetAltitude(10);
    const auto atEnd = test.map.getCameraOptions({});

    EXPECT_NEAR(later, settledAltitude, 0.5);
    EXPECT_NEAR(atEnd.center->latitude(), afterSettling.center->latitude(), 1e-6);
    EXPECT_NEAR(atEnd.center->longitude(), afterSettling.center->longitude(), 1e-6);
    EXPECT_NEAR(*atEnd.zoom, *afterSettling.zoom, 1e-6);
}

// Turning it off leaves the camera where the caller put it.
TEST(TerrainCamera, ClampingCanBeTurnedOff) {
    TerrainCameraTest test;
    test.map.setCenterClampedToGround(false);
    EXPECT_FALSE(test.map.getCenterClampedToGround());

    EXPECT_NEAR(test.renderAndGetAltitude(8), 0.0, 0.001);
}
