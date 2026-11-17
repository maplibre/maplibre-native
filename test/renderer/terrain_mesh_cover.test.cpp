#include <mln/test/util.hpp>

#include <mln/map/camera.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/util/tile_cover.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
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
    // A z12 DEM: Performance meshes to z14; Balanced follows the view, as Quality does.
    EXPECT_EQ(14, meshMaxZoom(TerrainLoadMode::Performance, 12));
    EXPECT_EQ(util::DEFAULT_MAX_ZOOM, meshMaxZoom(TerrainLoadMode::Balanced, 12));
}

TEST(TerrainMeshCover, ADeepDEMGetsFewOrNoExtraLevels) {
    // A z15 DEM (lidar) is already as deep as Performance goes.
    EXPECT_EQ(15, meshMaxZoom(TerrainLoadMode::Performance, 15));
}

TEST(TerrainMeshCover, QualityAndBalancedFollowTheViewAsGlJsDoes) {
    // maplibre-gl-js's TerrainSourceCache covers to z22 whatever the DEM's maxzoom, clamping only
    // the DEM lookup; in Quality and Balanced only the tile cap (and the coarsening before it)
    // bounds the mesh.
    for (const auto mode : {TerrainLoadMode::Quality, TerrainLoadMode::Balanced}) {
        for (const uint8_t demMaxZoom : {6, 12, 15, 18, 22}) {
            EXPECT_EQ(util::DEFAULT_MAX_ZOOM, meshMaxZoom(mode, demMaxZoom));
        }
    }
}

TEST(TerrainMeshCover, MeshIsNeverShallowerThanTheDEM) {
    for (const auto mode : allModes) {
        EXPECT_LE(18, meshMaxZoom(mode, 18));
    }
    EXPECT_EQ(18, meshMaxZoom(TerrainLoadMode::Performance, 18));
}

TEST(TerrainMeshCover, PerformanceGoesAtMostThreeLevelsPastTheDEM) {
    EXPECT_EQ(9, meshMaxZoom(TerrainLoadMode::Performance, 6));
}

namespace {
// An 8x8 block of z17 tiles. Its middle is the corner (4, 4) in tile units: the 16 tiles nearest
// it are the 4x4 block around it (squared distance up to 4.5 tile widths), the next 8 follow at 6.5
// and the next 8 at 8.5, so the 16th and the 24th nearest each end a group of tied distances.
std::set<UnwrappedTileID> z17Block() {
    std::set<UnwrappedTileID> cover;
    for (uint32_t y = 0; y < 8; ++y) {
        for (uint32_t x = 0; x < 8; ++x) cover.insert(UnwrappedTileID(17, 70000 + x, 40000 + y));
    }
    return cover;
}

// A point at (x, y) tile widths from the block's top left corner, in normalized web-mercator.
std::array<double, 2> z17Point(double x, double y) {
    const double n = std::exp2(17);
    return {{(70000 + x) / n, (40000 + y) / n}};
}

// Squared distance, in z17 tile widths, from the block's middle corner to the tile's centre.
double fromMiddle2(const UnwrappedTileID& id) {
    const double dx = static_cast<double>(id.canonical.x) - 70000 + 0.5 - 4;
    const double dy = static_cast<double>(id.canonical.y) - 40000 + 0.5 - 4;
    return dx * dx + dy * dy;
}

std::set<UnwrappedTileID> nearest16() {
    std::set<UnwrappedTileID> tiles;
    for (const auto& id : z17Block()) {
        if (fromMiddle2(id) < 5.0) tiles.insert(id);
    }
    return tiles;
}
} // namespace

TEST(TerrainMeshCover, TheNearestTilesOfTheFinestZoomGetFullSizeDrapes) {
    // A coarser tile west of the block (z15 17499/10000 is z17 69996-69999 / 40000-40003), with the
    // camera at its middle: the nearest tile of all, yet not of the finest zoom, so never full size.
    auto cover = z17Block();
    const UnwrappedTileID coarse(15, 17499, 10000);
    cover.insert(coarse);

    const auto full = RenderTerrain::fullSizeDrapeTilesOf(cover, z17Point(-2, 2), {});
    EXPECT_EQ(RenderTerrain::fullSizeDrapeNearest, full.size());
    EXPECT_FALSE(full.contains(coarse));
    for (const auto& id : full) EXPECT_EQ(17, id.canonical.z);

    EXPECT_TRUE(RenderTerrain::fullSizeDrapeTilesOf({}, z17Point(-2, 2), {}).empty());
}

TEST(TerrainMeshCover, TiedDistancesGoByTileIDForFullSizeDrapes) {
    // A row of z17 tiles with the camera over the middle one: the others pair off at equal
    // distances either side, and the 16th and 17th nearest are such a pair.
    std::set<UnwrappedTileID> row;
    for (uint32_t x = 0; x < 21; ++x) row.insert(UnwrappedTileID(17, 70000 + x, 40000));

    const auto full = RenderTerrain::fullSizeDrapeTilesOf(row, z17Point(10.5, 0.5), {});
    EXPECT_EQ(RenderTerrain::fullSizeDrapeNearest, full.size());
    EXPECT_TRUE(full.contains(UnwrappedTileID(17, 70002, 40000)));
    EXPECT_FALSE(full.contains(UnwrappedTileID(17, 70018, 40000)));
}

TEST(TerrainMeshCover, ATileKeepsItsFullSizeDrapeUntilTheMostAreNearer) {
    const auto cover = z17Block();
    const auto origin = z17Point(4, 4);

    // A tile just past the nearest 16 and the far corner had full size: both keep it, with room
    // to spare.
    const UnwrappedTileID twentieth(17, 70001, 40003);
    const UnwrappedTileID corner(17, 70000, 40000);
    ASSERT_EQ(6.5, fromMiddle2(twentieth));
    auto expected = nearest16();
    expected.insert(twentieth);
    expected.insert(corner);
    EXPECT_EQ(expected, RenderTerrain::fullSizeDrapeTilesOf(cover, origin, {twentieth, corner}));

    // All of them had: the nearest 24 keep it, and nothing past them, whatever it had.
    const auto kept = RenderTerrain::fullSizeDrapeTilesOf(cover, origin, cover);
    EXPECT_EQ(RenderTerrain::fullSizeDrapeMax, kept.size());
    for (const auto& id : cover) {
        EXPECT_EQ(fromMiddle2(id) <= 6.5, kept.contains(id)) << id.canonical.x << "/" << id.canonical.y;
    }
}

TEST(TerrainMeshCover, TilesKeepTheirFullSizeDrapesWhenTheFinestZoomChanges) {
    // The z17 tile beside the camera is replaced by its four z18 children, and z17 is no longer the
    // finest zoom. The other z17 tiles that had full-size targets keep them (the swap would re-bake
    // them all at once), and the z18 tiles, now the finest, get theirs too.
    auto cover = z17Block();
    const auto origin = z17Point(4, 4);
    const auto hadFullSize = RenderTerrain::fullSizeDrapeTilesOf(cover, origin, {});
    ASSERT_EQ(nearest16(), hadFullSize);

    const UnwrappedTileID parent(17, 70003, 40003);
    ASSERT_TRUE(hadFullSize.contains(parent));
    std::set<UnwrappedTileID> children;
    for (uint32_t y = 80006; y < 80008; ++y) {
        for (uint32_t x = 140006; x < 140008; ++x) children.insert(UnwrappedTileID(18, x, y));
    }
    cover.erase(parent);
    cover.insert(children.begin(), children.end());

    auto expected = hadFullSize;
    expected.erase(parent);
    expected.insert(children.begin(), children.end());
    EXPECT_EQ(expected, RenderTerrain::fullSizeDrapeTilesOf(cover, origin, hadFullSize));

    // They leave again: the same tiles, no change.
    for (const auto& child : children) cover.erase(child);
    cover.insert(parent);
    EXPECT_EQ(hadFullSize, RenderTerrain::fullSizeDrapeTilesOf(cover, origin, hadFullSize));
}

TEST(TerrainMeshCover, CoarserTilesThatHadFullSizeNeverDisplaceTheNearestFinestOnes) {
    // The camera is ten tiles west of the block, and twelve z16 tiles around it (z17 69986-69991 /
    // 40000-40007) had full-size targets: the finest zoom was z16 a moment ago. They are all nearer
    // than the block, but its nearest 16 tiles, its two western columns, still get theirs.
    auto cover = z17Block();
    std::set<UnwrappedTileID> hadFullSize;
    for (uint32_t y = 20000; y < 20004; ++y) {
        for (uint32_t x = 34993; x < 34996; ++x) hadFullSize.insert(UnwrappedTileID(16, x, y));
    }
    cover.insert(hadFullSize.begin(), hadFullSize.end());

    const auto full = RenderTerrain::fullSizeDrapeTilesOf(cover, z17Point(-10, 4), hadFullSize);
    EXPECT_EQ(RenderTerrain::fullSizeDrapeMax, full.size());
    for (uint32_t y = 40000; y < 40008; ++y) {
        EXPECT_TRUE(full.contains(UnwrappedTileID(17, 70000, y)));
        EXPECT_TRUE(full.contains(UnwrappedTileID(17, 70001, y)));
    }
}

TEST(TerrainMeshCover, AStillTakesTheMaxFullSizeDrapesAtOnce) {
    // A still has no earlier frame to keep steady: the nearest 24 tiles at full size (the same
    // bound as a moving view), not the 16 a moving view starts from.
    const auto cover = z17Block();
    const auto full = RenderTerrain::fullSizeDrapeTilesOf(cover, z17Point(4, 4), {}, RenderTerrain::fullSizeDrapeMax);
    EXPECT_EQ(RenderTerrain::fullSizeDrapeMax, full.size());
    for (const auto& id : cover) {
        EXPECT_EQ(fromMiddle2(id) <= 6.5, full.contains(id)) << id.canonical.x << "/" << id.canonical.y;
    }
}

TEST(TerrainMeshCover, AStillOnAReusedRendererIsDecidedFromScratch) {
    // A view of nine z17 tiles and three coarser z16 tiles beside them. The renderer drew other
    // views before, and its pool still holds their sizes: the three z16 tiles have full-size targets.
    // A moving map keeps those (there is room); a still is the same on this renderer as on a fresh
    // one: the nearest tiles of the finest zoom, and no coarse tile at full size.
    std::set<UnwrappedTileID> cover;
    for (uint32_t y = 0; y < 3; ++y) {
        for (uint32_t x = 0; x < 3; ++x) cover.insert(UnwrappedTileID(17, 70000 + x, 40000 + y));
    }
    const std::set<UnwrappedTileID> coarse{
        UnwrappedTileID(16, 34997, 20000), UnwrappedTileID(16, 34997, 20001), UnwrappedTileID(16, 34998, 20002)};
    cover.insert(coarse.begin(), coarse.end());
    const auto origin = z17Point(1.5, 1.5);

    const auto moving = RenderTerrain::drapeSizesFor(MapMode::Continuous, 16, cover, origin, coarse);
    EXPECT_EQ(12u, moving.fullSizeTiles.size());

    for (const auto mode : {MapMode::Static, MapMode::Tile}) {
        const auto fresh = RenderTerrain::drapeSizesFor(mode, 16, cover, origin, {});
        const auto reused = RenderTerrain::drapeSizesFor(mode, 16, cover, origin, coarse);
        EXPECT_EQ(9u, reused.fullSizeTiles.size());
        EXPECT_EQ(fresh.fullSizeTiles, reused.fullSizeTiles);
        for (const auto& id : reused.fullSizeTiles) EXPECT_EQ(17, id.canonical.z);
        // No swaps are left for a later frame: a still gets no follow-up to finish deferred ones.
        EXPECT_EQ(0u, fresh.maxResizes);
        EXPECT_EQ(0u, reused.maxResizes);
    }
}

TEST(TerrainMeshCover, AMovingMapKeepsItsSizesSteadyAndPacesTheSwaps) {
    // The same pool in a map that keeps moving: the earlier sizes count, and the drape cap paces
    // the swaps (Quality's 0 stays unlimited).
    const auto cover = z17Block();
    const auto origin = z17Point(4, 4);
    const UnwrappedTileID kept(17, 70007, 40007); // far corner, had full size
    const auto moving = RenderTerrain::drapeSizesFor(MapMode::Continuous, 16, cover, origin, {kept});

    auto expected = nearest16();
    expected.insert(kept);
    EXPECT_EQ(expected, moving.fullSizeTiles);
    EXPECT_EQ(16u, moving.maxResizes);
    EXPECT_EQ(0u, RenderTerrain::drapeSizesFor(MapMode::Continuous, 0, cover, origin, {}).maxResizes);
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

TEST(TerrainMeshCover, CoarseningAWideCoverIsFast) {
    // A camera high over rising ground puts thousands of tiles in the cover (6,000 in a test
    // view), far more than the cap. Merging them took a pass over the whole cover for each merge,
    // seconds a frame. The 16,384 z7 tiles of the world go down to 64 in well under a second.
    std::set<UnwrappedTileID> original;
    for (uint32_t y = 0; y < 128; ++y) {
        for (uint32_t x = 0; x < 128; ++x) {
            original.insert(tile(7, x, y));
        }
    }
    const auto start = std::chrono::steady_clock::now();
    const auto coarse = RenderTerrain::coarsenFarMeshTiles(original, 64, 0, northWestCorner);
    const auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_LE(coarse.size(), 64u);
    EXPECT_TRUE(coversAll(coarse, original));
    EXPECT_LT(elapsed, std::chrono::seconds(2)) << "coarsening the cover took too long";
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

namespace {

// Elevation a flat plateau `meters` up, for a cover that is tested against terrain.
struct Plateau final : util::TileElevationProvider {
    explicit Plateau(double meters_)
        : meters(meters_) {}
    std::optional<Range<double>> getTileElevationRange(const CanonicalTileID&) const override {
        return Range<double>{meters, meters};
    }
    double meters;
};

// The zoom of the cover tile whose centre lies nearest `origin` (normalized web-mercator).
int zoomOfTileNearest(const std::vector<OverscaledTileID>& cover, const std::array<double, 2>& origin) {
    int zoom = -1;
    double nearest = std::numeric_limits<double>::max();
    for (const auto& id : cover) {
        const double scale = std::exp2(id.canonical.z);
        const double distance = std::hypot((id.canonical.x + 0.5) / scale - origin[0],
                                           (id.canonical.y + 0.5) / scale - origin[1]);
        if (distance < nearest) {
            nearest = distance;
            zoom = id.canonical.z;
        }
    }
    return zoom;
}

} // namespace

TEST(TerrainMeshCover, TheTilesUnderATiltedCameraKeepTheFinestZoom) {
    // A phone-sized view over a plateau 2800 m up, tilted to 80 degrees. The ground at the bottom of
    // the screen is the biggest on it, and the level of detail has to keep it at the zoom asked for.
    // Measured from where the centre ray meets sea level, tens of tiles past the plateau, it left the
    // tile nearest the camera four levels coarser (z12 at z16 asked for), which the mesh then draped
    // from 16 times fewer texels.
    Transform transform;
    transform.resize({402, 874});
    transform.setMaxPitch(85.0);
    transform.jumpTo(
        CameraOptions().withCenter(LatLng{38.684, -120.091}).withZoom(15.0).withPitch(80.0).withCenterAltitude(2800.0));
    const Plateau plateau(2800.0);
    const std::array<double, 2> underTheCamera = RenderTerrain::meshTileCapOrigin(transform.getState());

    util::TileCoverParameters parameters{.transformState = transform.getState(), .elevationProvider = &plateau};
    parameters.tileLodOrigin = underTheCamera;
    const auto cover = util::tileCover(parameters, 16, Range<uint8_t>{0, 16}, uint8_t(16));
    EXPECT_EQ(16, zoomOfTileNearest(cover, underTheCamera));
}

TEST(TerrainMeshCover, DilationSkipsNeighboursOverTilesTheCoverHas) {
    // A z2 tile with a z3 tile beside it, to its east: the z2 tile's east neighbour is the z2 tile
    // over the z3 one, and the z3 tile's west neighbours are inside the z2 tile. None of them is
    // new ground; each would be a second surface over ground that has one.
    const UnwrappedTileID coarse(0, CanonicalTileID(2, 1, 1));
    const UnwrappedTileID fine(0, CanonicalTileID(3, 4, 2));
    const std::set<UnwrappedTileID> dilated = RenderTerrain::dilateCover({coarse, fine});

    for (const auto& a : dilated) {
        for (const auto& b : dilated) {
            EXPECT_FALSE(a != b && (a.isChildOf(b) || b.isChildOf(a)))
                << "z" << int(a.canonical.z) << "/" << a.canonical.x << "/" << a.canonical.y << " and z"
                << int(b.canonical.z) << "/" << b.canonical.x << "/" << b.canonical.y << " overlap";
        }
    }
    // The ground the cover leaves out is still taken: the z2 tile to the north of the coarse one.
    EXPECT_TRUE(dilated.contains(UnwrappedTileID(0, CanonicalTileID(2, 1, 0))));
    EXPECT_TRUE(dilated.contains(coarse));
    EXPECT_TRUE(dilated.contains(fine));
}
