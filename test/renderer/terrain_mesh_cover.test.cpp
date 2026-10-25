#include <mln/test/util.hpp>

#include <mln/map/camera.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>

#include <algorithm>
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

// A 512 px map centred on the equator at 90°E (x = 0.75, y = 0.5 in normalized mercator).
void look(Transform& transform, double pitch, double bearing) {
    transform.resize({512, 512});
    transform.jumpTo(
        CameraOptions().withCenter(LatLng{0.0, 90.0}).withZoom(10.0).withPitch(pitch).withBearing(bearing));
}

// The camera's position from the free-camera API: where the ground under it is, worked out
// independently of RenderTerrain.
std::array<double, 2> cameraGround(const Transform& transform) {
    const vec3 position = *transform.getFreeCameraOptions().position;
    return {{position[0], position[1]}};
}

UnwrappedTileID z10At(double x, double y) {
    return UnwrappedTileID(0, CanonicalTileID(10, static_cast<uint32_t>(x * 1024), static_cast<uint32_t>(y * 1024)));
}
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

TEST(TerrainMeshCover, CapOriginIsTheGroundUnderATiltedCamera) {
    for (const double bearing : {0.0, 90.0, 225.0}) {
        Transform transform;
        look(transform, 60.0, bearing);

        const auto origin = RenderTerrain::meshTileCapOrigin(transform.getState());
        const auto camera = cameraGround(transform);
        // 1e-7 of the world is a few metres, well inside a z10 tile.
        EXPECT_NEAR(camera[0], origin[0], 1e-7) << "bearing " << bearing;
        EXPECT_NEAR(camera[1], origin[1], 1e-7) << "bearing " << bearing;
    }
}

TEST(TerrainMeshCover, CapKeepsTheTileUnderATiltedCamera) {
    // Facing north at a 60° tilt, the camera stands south of the centre. Of two tiles as far
    // from the centre, one under the camera and one as far ahead of the centre, the cap keeps
    // the one under the camera: the biggest on screen, where the other is near the horizon.
    Transform transform;
    look(transform, 60.0, 0.0);
    const auto origin = RenderTerrain::meshTileCapOrigin(transform.getState());
    const double behind = origin[1] - 0.5;
    ASSERT_GT(behind, 1.0 / 1024) << "the camera should stand more than a z10 tile behind the centre";

    const UnwrappedTileID underTheCamera = z10At(0.75, 0.5 + behind);
    const UnwrappedTileID ahead = z10At(0.75, 0.5 - behind);
    const std::set<UnwrappedTileID> kept = RenderTerrain::nearestMeshTiles({underTheCamera, ahead}, 1, origin);
    EXPECT_EQ(std::set<UnwrappedTileID>{underTheCamera}, kept);
}

namespace {
int meshMaxZoom(TerrainLoadMode mode, uint8_t demMaxZoom) {
    return RenderTerrain::meshMaxZoom(mode, demMaxZoom);
}

constexpr TerrainLoadMode allModes[] = {
    TerrainLoadMode::Quality, TerrainLoadMode::Balanced, TerrainLoadMode::Performance};
} // namespace

TEST(TerrainMeshCover, MeshGoesPastACoarseDEMAsFarAsTheModeAffords) {
    // A z12 DEM: Performance meshes to z14 and Balanced to z15. Quality's z16 would be four
    // levels past the DEM, one more than the limit.
    EXPECT_EQ(14, meshMaxZoom(TerrainLoadMode::Performance, 12));
    EXPECT_EQ(15, meshMaxZoom(TerrainLoadMode::Balanced, 12));
    EXPECT_EQ(15, meshMaxZoom(TerrainLoadMode::Quality, 12));
}

TEST(TerrainMeshCover, ADeepDEMGetsFewOrNoExtraLevels) {
    // A z15 DEM (lidar) is already as deep as Performance and Balanced go; Quality adds one.
    EXPECT_EQ(15, meshMaxZoom(TerrainLoadMode::Performance, 15));
    EXPECT_EQ(15, meshMaxZoom(TerrainLoadMode::Balanced, 15));
    EXPECT_EQ(16, meshMaxZoom(TerrainLoadMode::Quality, 15));
}

TEST(TerrainMeshCover, MeshIsNeverShallowerThanTheDEM) {
    for (const auto mode : allModes) {
        EXPECT_EQ(18, meshMaxZoom(mode, 18));
    }
}

TEST(TerrainMeshCover, MeshGoesAtMostThreeLevelsPastTheDEM) {
    for (const auto mode : allModes) {
        EXPECT_EQ(9, meshMaxZoom(mode, 6));
    }
}

namespace {
UnwrappedTileID tile(uint8_t z, uint32_t x, uint32_t y) {
    return UnwrappedTileID(0, CanonicalTileID(z, x, y));
}

// Whether every tile of `original` lies inside a tile of `cover`.
bool coversAll(const std::set<UnwrappedTileID>& cover, const std::set<UnwrappedTileID>& original) {
    return std::ranges::all_of(original, [&](const UnwrappedTileID& id) {
        return std::ranges::any_of(cover, [&](const UnwrappedTileID& c) { return c == id || id.isChildOf(c); });
    });
}

constexpr std::array<double, 2> northWestCorner{{0.0, 0.0}};
} // namespace

TEST(TerrainMeshCover, CoarseningLeavesACoverWithinTheCapAlone) {
    EXPECT_EQ(wholeZ2(), RenderTerrain::coarsenFarMeshTiles(wholeZ2(), 16, 0, worldCentre));
    EXPECT_EQ(wholeZ2(), RenderTerrain::coarsenFarMeshTiles(wholeZ2(), 0, 0, worldCentre));
}

TEST(TerrainMeshCover, CoarseningMergesTheFarthestTilesFirst) {
    // Seen from the north-west corner, the south-east quarter is farthest: its four z2 tiles
    // become their z1 parent (13 tiles). Then the south-west and north-east quarters lie as far;
    // the lower ID, z1 (0,1), merges, which leaves 10. The northern half, nearer the corner,
    // keeps all its z2 tiles.
    const std::set<UnwrappedTileID> expected{tile(1, 1, 1),
                                             tile(1, 0, 1),
                                             tile(2, 0, 0),
                                             tile(2, 1, 0),
                                             tile(2, 2, 0),
                                             tile(2, 3, 0),
                                             tile(2, 0, 1),
                                             tile(2, 1, 1),
                                             tile(2, 2, 1),
                                             tile(2, 3, 1)};
    EXPECT_EQ(expected, RenderTerrain::coarsenFarMeshTiles(wholeZ2(), 10, 1, northWestCorner));
}

TEST(TerrainMeshCover, CoarseningNeverGoesShallowerThanTheMinimumZoom) {
    // All tiles already at the minimum zoom: nothing changes, and the cap is left to drop tiles.
    EXPECT_EQ(wholeZ2(), RenderTerrain::coarsenFarMeshTiles(wholeZ2(), 4, 2, worldCentre));
}

TEST(TerrainMeshCover, CoarseningKeepsAllTheGroundCoveredWithinTheCap) {
    // The 64 z4 tiles of z1 tile (0,0), seen from the north-west corner, down to 12 tiles, no
    // coarser than z2 (like a mesh three levels past a z2 DEM).
    std::set<UnwrappedTileID> original;
    for (uint32_t y = 0; y < 8; ++y) {
        for (uint32_t x = 0; x < 8; ++x) {
            original.insert(tile(4, x, y));
        }
    }
    const auto coarse = RenderTerrain::coarsenFarMeshTiles(original, 12, 2, northWestCorner);
    EXPECT_LE(coarse.size(), 12u);
    EXPECT_TRUE(coversAll(coarse, original));
    EXPECT_TRUE(coarse.contains(tile(4, 0, 0))) << "the tile nearest the camera keeps its zoom";
    // Detail falls off with distance: the four z4 tiles at the corner, z3 around them, z2 beyond.
    const std::set<UnwrappedTileID> expected{tile(4, 0, 0),
                                             tile(4, 1, 0),
                                             tile(4, 0, 1),
                                             tile(4, 1, 1),
                                             tile(3, 1, 0),
                                             tile(3, 0, 1),
                                             tile(3, 1, 1),
                                             tile(2, 1, 0),
                                             tile(2, 0, 1),
                                             tile(2, 1, 1)};
    EXPECT_EQ(expected, coarse);
}

TEST(TerrainMeshCover, CoarseningHandlesTilesWithoutTheirSiblings) {
    // The lone far tile becomes its parent, then its grandparent, without the count changing;
    // then the two near siblings merge, and the result fits.
    const std::set<UnwrappedTileID> original{tile(3, 0, 0), tile(3, 1, 0), tile(3, 7, 7)};
    const auto coarse = RenderTerrain::coarsenFarMeshTiles(original, 2, 1, northWestCorner);
    EXPECT_EQ((std::set<UnwrappedTileID>{tile(2, 0, 0), tile(1, 1, 1)}), coarse);
    EXPECT_TRUE(coversAll(coarse, original));
}

namespace {
// Where the DEM mapping puts a tile-local position, in the DEM tile's normalized coordinates.
std::array<float, 2> inDEM(const std::array<float, 3>& mapping, float x, float y) {
    return {{x * mapping[0] + mapping[1], y * mapping[0] + mapping[2]}};
}
} // namespace

TEST(TerrainMeshCover, ATileMapsOntoItsOwnDEMWhole) {
    const CanonicalTileID tile(12, 2200, 1400);
    const auto mapping = RenderTerrain::demTileMapping(tile, tile);
    EXPECT_EQ((std::array<float, 2>{{0.0f, 0.0f}}), inDEM(mapping, 0, 0));
    EXPECT_EQ((std::array<float, 2>{{1.0f, 1.0f}}), inDEM(mapping, util::EXTENT, util::EXTENT));
}

TEST(TerrainMeshCover, TilesPastTheDEMMapOntoTheirSubSquareOfTheAncestor) {
    // A mesh three levels past a z12 DEM: every descendant one to three levels down maps exactly
    // onto its own sub-square of the z12 tile, so neighbours meet without gaps or overlap.
    const CanonicalTileID dem(12, 2200, 1400);
    for (uint8_t depth = 1; depth <= 3; ++depth) {
        const uint32_t n = 1u << depth;
        for (uint32_t cy = 0; cy < n; ++cy) {
            for (uint32_t cx = 0; cx < n; ++cx) {
                const CanonicalTileID tile(12 + depth, (dem.x << depth) + cx, (dem.y << depth) + cy);
                const auto mapping = RenderTerrain::demTileMapping(tile, dem);
                const float side = 1.0f / static_cast<float>(n);
                EXPECT_EQ((std::array<float, 2>{{cx * side, cy * side}}), inDEM(mapping, 0, 0))
                    << "depth " << int(depth) << ", child " << cx << "," << cy;
                EXPECT_EQ((std::array<float, 2>{{(cx + 1) * side, (cy + 1) * side}}),
                          inDEM(mapping, util::EXTENT, util::EXTENT))
                    << "depth " << int(depth) << ", child " << cx << "," << cy;
            }
        }
    }
}
