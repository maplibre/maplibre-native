#include <mln/test/util.hpp>

#include <mln/renderer/render_terrain.hpp>
#include <mln/tile/tile_id.hpp>

#include <set>

using namespace mln;

namespace {
constexpr uint8_t north = 1;
constexpr uint8_t east = 2;
constexpr uint8_t south = 4;
constexpr uint8_t west = 8;

UnwrappedTileID tile(uint8_t z, uint32_t x, uint32_t y, int16_t wrap = 0) {
    return UnwrappedTileID(wrap, CanonicalTileID(z, x, y));
}

int edges(const UnwrappedTileID& id, const std::set<UnwrappedTileID>& cover) {
    return RenderTerrain::skirtEdges(id, cover);
}
} // namespace

TEST(TerrainSkirtEdges, LoneTileHasNone) {
    const std::set<UnwrappedTileID> cover{tile(4, 5, 5)};
    EXPECT_EQ(0, edges(tile(4, 5, 5), cover));
}

TEST(TerrainSkirtEdges, SameZoomNeighborsCountOnTheEdgeTheyTouch) {
    const std::set<UnwrappedTileID> cover{tile(4, 5, 5), tile(4, 6, 5), tile(4, 7, 5), tile(4, 6, 6)};
    EXPECT_EQ(east, edges(tile(4, 5, 5), cover));
    EXPECT_EQ(west | east | south, edges(tile(4, 6, 5), cover));
    EXPECT_EQ(west, edges(tile(4, 7, 5), cover));
    EXPECT_EQ(north, edges(tile(4, 6, 6), cover));
}

TEST(TerrainSkirtEdges, DiagonalNeighborsDoNotCount) {
    const std::set<UnwrappedTileID> cover{tile(4, 5, 5), tile(4, 6, 6), tile(4, 4, 4)};
    EXPECT_EQ(0, edges(tile(4, 5, 5), cover));
}

TEST(TerrainSkirtEdges, CoarserNeighborCoversTheAdjacentArea) {
    // The zoom 4 tile (6, 5) is the zoom 5 area x 12-13, y 10-11, so it lies east of both zoom 5 tiles
    // at x = 11.
    const std::set<UnwrappedTileID> cover{tile(4, 6, 5), tile(5, 11, 10), tile(5, 11, 11), tile(5, 10, 10)};
    EXPECT_EQ(east | south | west, edges(tile(5, 11, 10), cover));
    EXPECT_EQ(north | east, edges(tile(5, 11, 11), cover));
    EXPECT_EQ(east, edges(tile(5, 10, 10), cover));
}

TEST(TerrainSkirtEdges, FinerNeighborsCountWhereTheyTouchTheEdge) {
    const std::set<UnwrappedTileID> touching{tile(4, 6, 5), tile(5, 11, 10)};
    EXPECT_EQ(west, edges(tile(4, 6, 5), touching));
    EXPECT_EQ(east, edges(tile(5, 11, 10), touching));

    // Two zooms finer, and only the tile at the edge counts: x = 23 touches, x = 22 does not.
    const std::set<UnwrappedTileID> deep{tile(4, 6, 5), tile(6, 23, 20)};
    EXPECT_EQ(west, edges(tile(4, 6, 5), deep));
    const std::set<UnwrappedTileID> apart{tile(4, 6, 5), tile(6, 22, 20)};
    EXPECT_EQ(0, edges(tile(4, 6, 5), apart));
    EXPECT_EQ(0, edges(tile(6, 22, 20), apart));
}

TEST(TerrainSkirtEdges, FrontierEdgesHaveNone) {
    // A row of three above a lone tile: the outer edges of the group, and the gap beside the lone tile,
    // are frontier.
    const std::set<UnwrappedTileID> cover{tile(5, 10, 10), tile(5, 11, 10), tile(5, 12, 10), tile(5, 11, 12)};
    EXPECT_EQ(east, edges(tile(5, 10, 10), cover));
    EXPECT_EQ(east | west, edges(tile(5, 11, 10), cover));
    EXPECT_EQ(west, edges(tile(5, 12, 10), cover));
    EXPECT_EQ(0, edges(tile(5, 11, 12), cover));
}

TEST(TerrainSkirtEdges, TheTopOfTheWorldHasNoNorthNeighbor) {
    const std::set<UnwrappedTileID> cover{tile(3, 2, 0), tile(3, 2, 1)};
    EXPECT_EQ(south, edges(tile(3, 2, 0), cover));
    EXPECT_EQ(north, edges(tile(3, 2, 1), cover));
}

TEST(TerrainSkirtEdges, NeighborsAcrossTheAntimeridianCount) {
    const std::set<UnwrappedTileID> cover{tile(2, 3, 1, 0), tile(2, 0, 1, 1), tile(2, 0, 1, 0)};
    EXPECT_EQ(east, edges(tile(2, 3, 1, 0), cover));
    EXPECT_EQ(west, edges(tile(2, 0, 1, 1), cover));
    EXPECT_EQ(0, edges(tile(2, 0, 1, 0), cover));
}
