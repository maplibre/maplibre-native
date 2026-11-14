#include <mln/test/util.hpp>

#include <mln/renderer/render_terrain.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>

using namespace mln;

namespace {
constexpr float extent = static_cast<float>(util::EXTENT);

// z2 has a 4x4 tile grid, small enough to reason about the edges by hand.
UnwrappedTileID z2(uint32_t x, uint32_t y, int16_t wrap = 0) {
    return UnwrappedTileID(wrap, CanonicalTileID(2, x, y));
}
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
