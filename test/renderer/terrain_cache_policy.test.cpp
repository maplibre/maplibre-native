#include <mln/test/util.hpp>

#include <mln/renderer/terrain_cache_policy.hpp>

using namespace mln::terrain;

TEST(TerrainCachePolicy, RebindsToABetterTier) {
    EXPECT_TRUE(shouldRebind({0, -1}, {1, 6}));  // placeholder -> ancestor
    EXPECT_TRUE(shouldRebind({1, 12}, {2, 13})); // ancestor -> own DEM
    EXPECT_TRUE(shouldRebind({0, -1}, {2, 12})); // placeholder -> own DEM
}

TEST(TerrainCachePolicy, NeverRebindsToAWorseTier) {
    EXPECT_FALSE(shouldRebind({2, 12}, {1, 11}));
    EXPECT_FALSE(shouldRebind({1, 6}, {0, -1}));
}

TEST(TerrainCachePolicy, RebindsToADeeperAncestor) {
    // The regression: a z13 tile first built on the z6 ancestor stayed flat after z12 loaded.
    EXPECT_TRUE(shouldRebind({1, 6}, {1, 12}));
    EXPECT_TRUE(shouldRebind({1, 11}, {1, 12}));
}

TEST(TerrainCachePolicy, KeepsTheSameOrADeeperAncestor) {
    EXPECT_FALSE(shouldRebind({1, 12}, {1, 12}));
    EXPECT_FALSE(shouldRebind({1, 12}, {1, 6})); // a deeper ancestor was evicted: keep what is bound
    EXPECT_FALSE(shouldRebind({2, 12}, {2, 12}));
    EXPECT_FALSE(shouldRebind({0, -1}, {0, -1}));
}

namespace {
std::vector<DrapeGroupFallback> groups(std::initializer_list<int16_t> deficits) {
    std::vector<DrapeGroupFallback> out;
    int32_t layerIndex = 0;
    for (const auto deficit : deficits) {
        out.push_back({layerIndex++, deficit});
    }
    return out;
}
} // namespace

TEST(TerrainCachePolicy, SameDrapeContentIsNotWorse) {
    EXPECT_FALSE(drapeFallbackStrictlyWorse(groups({0, 1, -1}), groups({0, 1, -1})));
}

TEST(TerrainCachePolicy, LosingContentIsWorse) {
    // A tile briefly dropped out of the render set: keep the bake instead of flashing it empty.
    EXPECT_TRUE(drapeFallbackStrictlyWorse(groups({0, -1, 0}), groups({0, 1, 0})));
}

TEST(TerrainCachePolicy, CoarserFallbackIsWorse) {
    EXPECT_TRUE(drapeFallbackStrictlyWorse(groups({0, 3, 0}), groups({0, 1, 0})));
}

TEST(TerrainCachePolicy, AnyGroupImprovingIsNotWorse) {
    // The regression: hillshade moves from a z6 to its z12 DEM (deficit 7 -> 1) while vector groups
    // fall back a level. Summed over groups the new content scored worse (10 -> 16 in the field
    // log) and the hillshade-less bake was kept for the life of the target.
    EXPECT_FALSE(drapeFallbackStrictlyWorse(groups({1, 3, 3, 3}), groups({7, 0, 0, 0})));
    // A group gaining content at all is an improvement too.
    EXPECT_FALSE(drapeFallbackStrictlyWorse(groups({1, 4}), groups({-1, 0})));
}

TEST(TerrainCachePolicy, DifferentGroupSetsAreNotComparable) {
    EXPECT_FALSE(drapeFallbackStrictlyWorse(groups({-1, -1}), groups({0, 0, 0})));
    auto renumbered = groups({-1, -1});
    renumbered[1].layerIndex = 7;
    EXPECT_FALSE(drapeFallbackStrictlyWorse(renumbered, groups({0, 0})));
}

namespace {
// A z13 drape target over the Alps, its parent and one of its children
const mln::UnwrappedTileID target{13, 4355, 2896};
const mln::UnwrappedTileID parent{12, 2177, 1448};
const mln::UnwrappedTileID child{14, 8710, 5792};

std::vector<DrapeGroupFallback> fallbackOf(const DrapeGroupCoverage& group) {
    return {group.fallback()};
}
} // namespace

TEST(TerrainCachePolicy, DrapeGroupIgnoresTilesOutsideTheTarget) {
    DrapeGroupCoverage group(target, 0);
    group.addDrawn({13, 4356, 2896}, 1); // a neighbour
    group.addDrawn({14, 8712, 5792}, 1); // a neighbour's child
    group.addEmpty({12, 2178, 1448});    // a neighbour's parent
    EXPECT_EQ(group.contentHash(), 0u);
    EXPECT_EQ(group.fallback().deficit, -1);
}

TEST(TerrainCachePolicy, DrapeGroupFallsBackToItsDeepestAncestor) {
    DrapeGroupCoverage group(target, 3);
    group.addDrawn({11, 1088, 724}, 1);
    group.addDrawn(parent, 1);
    EXPECT_EQ(group.fallback().layerIndex, 3);
    EXPECT_EQ(group.fallback().deficit, 1);
    group.addDrawn(child, 1);
    EXPECT_EQ(group.fallback().deficit, 0);
}

TEST(TerrainCachePolicy, DrapeContentDoesNotDependOnOrder) {
    DrapeGroupCoverage forward(target, 0);
    DrapeGroupCoverage backward(target, 0);
    const auto children = target.children();
    for (std::size_t i = 0; i < children.size(); ++i) {
        forward.addDrawn(children[i], i + 1);
        backward.addDrawn(children[children.size() - 1 - i], children.size() - i);
    }
    EXPECT_EQ(forward.contentHash(), backward.contentHash());
}

TEST(TerrainCachePolicy, NewDataUnderTheSameTilesIsNewContent) {
    // The regression: a GeoJSON source updated with setGeoJson rebuilt its tiles from new buckets
    // under the same tile ids, and the drape kept the old route until the next zoom level.
    DrapeGroupCoverage before(target, 0);
    DrapeGroupCoverage after(target, 0);
    before.addDrawn(target, 1);
    after.addDrawn(target, 2);
    EXPECT_NE(after.contentHash(), before.contentHash());
    EXPECT_FALSE(drapeFallbackStrictlyWorse(fallbackOf(after), fallbackOf(before)));
}

TEST(TerrainCachePolicy, AnEmptiedTileIsNewContentNotLostContent) {
    // The regression: a route cleared from the map stayed drawn on the terrain. The layer has
    // no data on the tile any more, which the drape must show.
    DrapeGroupCoverage drawn(target, 0);
    DrapeGroupCoverage emptied(target, 0);
    drawn.addDrawn(target, 1);
    emptied.addEmpty(target);
    EXPECT_NE(emptied.contentHash(), drawn.contentHash());
    EXPECT_FALSE(drapeFallbackStrictlyWorse(fallbackOf(emptied), fallbackOf(drawn)));

    // And the same as long as it stays empty.
    DrapeGroupCoverage stillEmpty(target, 0);
    stillEmpty.addEmpty(target);
    EXPECT_EQ(stillEmpty.contentHash(), emptied.contentHash());
}

TEST(TerrainCachePolicy, ATileWithoutDrawablesYetKeepsTheBake) {
    // A tile still loading, or with its drawables deferred by the tile build budget, is neither
    // drawn nor empty: the group lost its content, and the drape keeps what it baked.
    DrapeGroupCoverage drawn(target, 0);
    DrapeGroupCoverage pending(target, 0);
    drawn.addDrawn(target, 1);
    EXPECT_TRUE(drapeFallbackStrictlyWorse(fallbackOf(pending), fallbackOf(drawn)));
}
