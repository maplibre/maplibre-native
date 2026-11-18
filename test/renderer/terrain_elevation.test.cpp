#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/style/style.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/image.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/run_loop.hpp>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <optional>
#include <thread>
#include <utility>

using namespace mln;

namespace {
constexpr float extent = static_cast<float>(util::EXTENT);

// z2 has a 4x4 tile grid, small enough to reason about the edges by hand.
UnwrappedTileID z2(uint32_t x, uint32_t y, int16_t wrap = 0) {
    return UnwrappedTileID(wrap, CanonicalTileID(2, x, y));
}

constexpr uint32_t rampTileSize = 64;

// The height of pixel (x, y) of the ramp tile in metres: a different whole number for every
// pixel, rising 10 m a column and 100 m a row.
double rampHeight(uint32_t x, uint32_t y) {
    return 1000.0 + 10.0 * x + 100.0 * y;
}

// The ramp as a Terrarium tile, which holds whole metres exactly: R * 256 + G - 32768.
std::string makeRampDEMTile() {
    PremultipliedImage image({rampTileSize, rampTileSize});
    for (uint32_t y = 0; y < rampTileSize; ++y) {
        for (uint32_t x = 0; x < rampTileSize; ++x) {
            const auto value = static_cast<uint32_t>(rampHeight(x, y)) + 32768;
            uint8_t* px = image.data.get() + (y * rampTileSize + x) * 4;
            px[0] = static_cast<uint8_t>(value >> 8);
            px[1] = static_cast<uint8_t>(value & 0xff);
            px[2] = 0;
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

const char* rampStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "terrarium",
      "maxzoom": 2,
      "tileSize": 64
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": []
})STYLE";
} // namespace

TEST(TerrainElevation, NormalizeLeavesInRangeCoordinatesAlone) {
    UnwrappedTileID tile = z2(1, 1);
    float x = 10.0f;
    float y = extent - 1.0f;

    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(1, 1));
    EXPECT_FLOAT_EQ(x, 10.0f);
    EXPECT_FLOAT_EQ(y, extent - 1.0f);
}

TEST(TerrainElevation, NormalizeCrossesTileEdges) {
    // East, past the right edge.
    UnwrappedTileID tile = z2(1, 1);
    float x = extent + 100.0f;
    float y = 50.0f;
    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(2, 1));
    EXPECT_FLOAT_EQ(x, 100.0f);
    EXPECT_FLOAT_EQ(y, 50.0f);

    // West, before the left edge.
    tile = z2(1, 1);
    x = -100.0f;
    y = 50.0f;
    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(0, 1));
    EXPECT_FLOAT_EQ(x, extent - 100.0f);

    // North, above the top edge.
    tile = z2(1, 1);
    x = 50.0f;
    y = -100.0f;
    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(1, 0));
    EXPECT_FLOAT_EQ(y, extent - 100.0f);

    // South, below the bottom edge.
    tile = z2(1, 1);
    x = 50.0f;
    y = extent + 100.0f;
    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(1, 2));
    EXPECT_FLOAT_EQ(y, 100.0f);
}

TEST(TerrainElevation, NormalizeCrossesMoreThanOneTile) {
    UnwrappedTileID tile = z2(1, 1);
    float x = 2.5f * extent;
    float y = 50.0f;

    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(3, 1));
    EXPECT_FLOAT_EQ(x, 0.5f * extent);
}

TEST(TerrainElevation, NormalizeWrapsAroundTheAntimeridian) {
    // Off the east edge of the world: same canonical column 0, one wrap further east.
    UnwrappedTileID tile = z2(3, 1);
    float x = extent + 50.0f;
    float y = 50.0f;
    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(0, 1, /*wrap=*/1));
    EXPECT_FLOAT_EQ(x, 50.0f);

    // Off the west edge, the other way.
    tile = z2(0, 1);
    x = -50.0f;
    y = 50.0f;
    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(3, 1, /*wrap=*/-1));
    EXPECT_FLOAT_EQ(x, extent - 50.0f);
}

TEST(TerrainElevation, NormalizeRejectsPastThePoles) {
    // North of the top row: the grid does not continue, so there is nothing to sample. The
    // inputs are left untouched so a caller can tell nothing was resolved.
    UnwrappedTileID tile = z2(1, 0);
    float x = 50.0f;
    float y = -50.0f;
    EXPECT_FALSE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(1, 0));
    EXPECT_FLOAT_EQ(y, -50.0f);

    // South of the bottom row.
    tile = z2(1, 3);
    x = 50.0f;
    y = extent + 50.0f;
    EXPECT_FALSE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(1, 3));
}

TEST(TerrainElevation, NormalizeKeepsTheResultInsideTheTile) {
    // A coordinate a hair below zero must not round back up onto the far edge, which belongs
    // to the next tile over.
    UnwrappedTileID tile = z2(1, 1);
    float x = -1e-9f;
    float y = 50.0f;

    EXPECT_TRUE(RenderTerrain::normalizeTileCoordinates(tile, x, y));
    EXPECT_EQ(tile, z2(0, 1));
    EXPECT_GE(x, 0.0f);
    EXPECT_LT(x, extent);
}

// A Terrain-RGB or Terrarium pixel holds the height of its whole cell, and hillshade and
// color-relief draw it over that cell, so the height at a pixel's centre is the pixel's value.
// Read as the height at the cell's top-left corner instead, the terrain stood half a pixel
// north-west of its data. The height is read through the centre altitude, which rides the ground
// under the map centre; the corner pixels' centres sit next to the tile's border.
TEST(TerrainElevation, APixelsHeightIsAtItsCentre) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string tile = makeRampDEMTile();
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
    map.getStyle().loadJSON(rampStyle);
    map.setCenterClampedToGround(true);

    for (const auto& [x, y] : {std::pair<uint32_t, uint32_t>{20, 40}, {0, 0}, {63, 63}}) {
        // The centre of pixel (x, y) of DEM tile 2/2/1, in z2 tile units.
        const Point<double> centre{2.0 + (x + 0.5) / rampTileSize, 1.0 + (y + 0.5) / rampTileSize};
        map.jumpTo(CameraOptions()
                       .withCenter(Projection::unproject(centre * util::tileSize_D, 4.0))
                       .withZoom(2.0)
                       .withPitch(0.0));
        // Until the centre altitude has the height, or for at most 100 frames: a slow machine can
        // render a few frames before the tile is in
        const auto centreAltitude = [&] {
            return map.getCameraOptions({}).centerAltitude.value_or(0.0);
        };
        for (int i = 0; i < 100 && std::abs(centreAltitude() - rampHeight(x, y)) > 0.5; ++i) {
            loop.runOnce();
            frontend.render(map);
        }
        EXPECT_NEAR(centreAltitude(), rampHeight(x, y), 0.5) << "pixel " << x << ", " << y;
    }
}

namespace {

// A Terrarium tile of one whole number of metres everywhere.
std::string makeUniformDEMTile(uint32_t metres) {
    const uint32_t value = metres + 32768;
    PremultipliedImage image({rampTileSize, rampTileSize});
    for (uint32_t i = 0; i < rampTileSize * rampTileSize; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = static_cast<uint8_t>(value >> 8);
        px[1] = static_cast<uint8_t>(value & 0xff);
        px[2] = 0;
        px[3] = 255;
    }
    return encodePNG(image);
}

} // namespace

// Within half a pixel of its edge a tile's height is read between its edge pixel and the border,
// which the neighbouring tile backfills when it loads. The map's copy of the heights
// (getTerrainElevation) must take that backfill too when the neighbour loads after the tile.
// Kept as first copied, it read the stand-in border (a copy of the tile's own edge) there,
// while the ground drawn on screen had the neighbour's.
TEST(TerrainElevation, ANeighbourLoadingLaterChangesTheHeightsNextToTheSharedEdge) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string evenColumn = makeUniformDEMTile(1000);
    const std::string oddColumn = makeUniformDEMTile(3000);
    // Tiles in odd columns are held back until released, so tile 2/2/1 is loaded before its west
    // neighbour 2/1/1
    bool holdOddColumns = true;
    bool oddRequested = false;
    fileSource->tileResponse = [&](const Resource& resource) -> std::optional<Response> {
        int z = 0, x = 0, y = 0;
        const auto name = resource.url.substr(resource.url.rfind('/') + 1);
        EXPECT_EQ(std::sscanf(name.c_str(), "%d-%d-%d.png", &z, &x, &y), 3) << resource.url;
        const bool odd = x % 2 == 1;
        if (odd) {
            oddRequested = true;
            if (holdOddColumns) {
                return std::nullopt;
            }
        }
        Response res;
        res.data = std::make_shared<std::string>(odd ? oddColumn : evenColumn);
        return res;
    };
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.getStyle().loadJSON(rampStyle);

    // A quarter pixel inside tile 2/2/1 from its west edge, on row 40's centre, in z2 tile units:
    // read with weight 1/4 from the border pixel and 3/4 from the tile's first pixel
    const LatLng nearTheEdge = Projection::unproject(
        Point<double>{2.0 + 0.25 / rampTileSize, 1.0 + 40.5 / rampTileSize} * util::tileSize_D, 4.0);
    map.jumpTo(CameraOptions().withCenter(nearTheEdge).withZoom(2.0).withPitch(0.0));

    // A Continuous map renders as tiles arrive. Renders until the height there has stayed the
    // same for a while
    const auto renderUntilSettled = [&] {
        constexpr int maxFrames = 200;
        constexpr int quietFrames = 10;
        std::optional<double> last;
        int quiet = 0;
        for (int frame = 0; frame < maxFrames && quiet < quietFrames; ++frame) {
            loop.runOnce();
            const auto elevation = map.getTerrainElevation(nearTheEdge);
            quiet = elevation && elevation == last ? quiet + 1 : 0;
            last = elevation;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        return last;
    };

    const auto before = renderUntilSettled();
    ASSERT_TRUE(oddRequested) << "the view has no tile in an odd column, so nothing backfills";
    ASSERT_TRUE(before) << "tile 2/2/1 has no height";
    EXPECT_NEAR(*before, 1000.0, 0.5) << "with its west neighbour not loaded, the border is the tile's own edge";

    holdOddColumns = false;
    const auto after = renderUntilSettled();
    ASSERT_TRUE(after);
    EXPECT_NEAR(*after, 0.75 * 1000.0 + 0.25 * 3000.0, 0.5)
        << "the map kept the heights of tile 2/2/1 from before its west neighbour backfilled its border";
}
