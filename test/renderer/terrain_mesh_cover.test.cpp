#include <mln/test/util.hpp>

#include <mln/map/camera.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/tile/tile_id.hpp>

#include <set>

using namespace mln;

namespace {
UnwrappedTileID z2(uint32_t x, uint32_t y) {
    return UnwrappedTileID(0, CanonicalTileID(2, x, y));
}

// The whole z2 world: a 4x4 grid, small enough to work out distances by hand.
std::set<UnwrappedTileID> wholeZ2() {
    std::set<UnwrappedTileID> tiles;
    for (uint32_t y = 0; y < 4; ++y) {
        for (uint32_t x = 0; x < 4; ++x) {
            tiles.insert(z2(x, y));
        }
    }
    return tiles;
}

constexpr std::array<double, 2> worldCentre{{0.5, 0.5}};
} // namespace

TEST(TerrainMeshCover, CapKeepsTheTilesNearestTheOrigin) {
    const std::set<UnwrappedTileID> middle{z2(1, 1), z2(2, 1), z2(1, 2), z2(2, 2)};
    EXPECT_EQ(middle, RenderTerrain::nearestMeshTiles(wholeZ2(), 4, worldCentre));
}

TEST(TerrainMeshCover, CapBreaksTiesByTileID) {
    // Past the middle four, the eight edge tiles beside them lie equally far from the centre;
    // the two with the lowest IDs stay, so the same view always keeps the same tiles.
    const std::set<UnwrappedTileID> expected{z2(1, 1), z2(2, 1), z2(1, 2), z2(2, 2), z2(0, 1), z2(0, 2)};
    EXPECT_EQ(expected, RenderTerrain::nearestMeshTiles(wholeZ2(), 6, worldCentre));
}

TEST(TerrainMeshCover, NoCapOrRoomToSpareKeepsEveryTile) {
    EXPECT_EQ(wholeZ2(), RenderTerrain::nearestMeshTiles(wholeZ2(), 0, worldCentre));
    EXPECT_EQ(wholeZ2(), RenderTerrain::nearestMeshTiles(wholeZ2(), 16, worldCentre));
    EXPECT_EQ(wholeZ2(), RenderTerrain::nearestMeshTiles(wholeZ2(), 64, worldCentre));
}

TEST(TerrainMeshCover, CapOriginWithoutTiltIsTheMapCentre) {
    Transform transform;
    transform.resize({512, 512});
    transform.jumpTo(CameraOptions().withCenter(LatLng{0.0, 90.0}).withZoom(10.0));

    const auto origin = RenderTerrain::meshTileCapOrigin(transform.getState());
    EXPECT_NEAR(0.75, origin[0], 1e-9);
    EXPECT_NEAR(0.5, origin[1], 1e-9);
}
