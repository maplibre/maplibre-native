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
        for (int i = 0; i < 8; ++i) {
            loop.runOnce();
            frontend.render(map);
        }
        EXPECT_NEAR(map.getCameraOptions({}).centerAltitude.value_or(0.0), rampHeight(x, y), 0.5)
            << "pixel " << x << ", " << y;
    }
}
