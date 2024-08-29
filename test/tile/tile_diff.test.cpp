#include <mbgl/test/util.hpp>
#include <mbgl/test/stub_file_source.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/tile/tile_diff.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/sources/render_tile_source.hpp>
#include <mbgl/renderer/buckets/debug_bucket.hpp>
#include <mbgl/renderer/tile_render_data.hpp>

#include <mbgl/test/source.test.hpp>
using namespace source_test;

using namespace mbgl;

namespace {

const struct Test {
    mbgl::style::VectorSource initialized{"source", Tileset{{"tiles"}}};
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>();
    TaggedScheduler threadPool{Scheduler::GetBackground(), {}};
    FakeTileSource source{initialized.baseImpl, threadPool};
    // As unwrapped, these are ordered by wrap and then z,x,y
    // As overscaled, they are ordered by overscaled-z (same as z), then wrap,z,x,y
    const std::vector<UnwrappedTileID> tileIDs{
        {0, {0, 0, 0}},
        {0, {1, 0, 0}},
        {0, {1, 1, 0}},
        {1, {0, 0, 0}},
        {1, {0, 0, 0}},
    };
    FakeTile tiles[5]{
        {source, tileIDs[0].overscaleTo(tileIDs[0].canonical.z)},
        {source, tileIDs[1].overscaleTo(tileIDs[1].canonical.z)},
        {source, tileIDs[2].overscaleTo(tileIDs[2].canonical.z)},
        {source, tileIDs[3].overscaleTo(tileIDs[3].canonical.z)},
        {source, tileIDs[4].overscaleTo(tileIDs[4].canonical.z)},
    };
    const RenderTile renderTiles[5]{
        {tileIDs[0], tiles[0]},
        {tileIDs[1], tiles[1]},
        {tileIDs[2], tiles[2]},
        {tileIDs[3], tiles[3]},
        {tileIDs[4], tiles[4]},
    };

    Test() { initialized.loadDescription(*fileSource); }
    template <typename T>
    auto renderTilesByIndex(T indexes) const {
        std::vector<RenderTiles::element_type::value_type> result;
        std::for_each(indexes.begin(), indexes.end(), [&](auto i) { result.push_back(renderTiles[i]); });
        return std::make_shared<std::vector<RenderTiles::element_type::value_type>>(result);
    }
} diffTest;

std::vector<OverscaledTileID> tileIDs(const RenderTiles& tiles) {
    std::vector<OverscaledTileID> ids;
    std::ranges::transform(
        *tiles, std::back_inserter(ids), [](const RenderTile& tile) { return tile.getOverscaledTileID(); });
    return ids;
}

} // namespace

TEST(TileDiff, Empty) {
    RenderTiles a = std::make_shared<RenderTiles::element_type>();
    RenderTiles b = std::make_shared<RenderTiles::element_type>();
    auto result = diffTiles(tileIDs(a), b);
    EXPECT_EQ(0, result.added.size());
    EXPECT_EQ(0, result.removed.size());
    EXPECT_EQ(0, result.remainder.size());
}

TEST(TileDiff, Add) {
    const auto a = diffTest.renderTilesByIndex(std::vector<int>{0, 1});
    const auto b = diffTest.renderTilesByIndex(std::vector<int>{0, 1, 2, 3});

    const auto result = diffTiles(tileIDs(a), b);
    EXPECT_EQ(2, result.added.size());
    EXPECT_EQ(&result.added[0].get(), &diffTest.renderTiles[3]);
    EXPECT_EQ(&result.added[1].get(), &diffTest.renderTiles[2]);
    EXPECT_EQ(0, result.removed.size());
    EXPECT_EQ(2, result.remainder.size());
    EXPECT_EQ(&result.remainder[0].get(), &diffTest.renderTiles[0]);
    EXPECT_EQ(&result.remainder[1].get(), &diffTest.renderTiles[1]);
}

TEST(TileDiff, Remove) {
    const auto a = diffTest.renderTilesByIndex(std::vector<int>{0, 1, 2, 3});
    const auto b = diffTest.renderTilesByIndex(std::vector<int>{0, 1});
    const auto result = diffTiles(tileIDs(a), b);
    EXPECT_EQ(0, result.added.size());
    EXPECT_EQ(2, result.removed.size());
    EXPECT_EQ(result.removed[0].toUnwrapped(), diffTest.tileIDs[3]);
    EXPECT_EQ(result.removed[1].toUnwrapped(), diffTest.tileIDs[2]);
    EXPECT_EQ(2, result.remainder.size());
}

TEST(TileDiff, Equal) {
    const auto a = diffTest.renderTilesByIndex(std::vector<int>{0, 1, 2, 3});
    const auto b = diffTest.renderTilesByIndex(std::vector<int>{0, 1, 2, 3});

    const auto result1 = diffTiles(tileIDs(a), b);
    EXPECT_EQ(0, result1.added.size());
    EXPECT_EQ(0, result1.removed.size());
    EXPECT_EQ(4, result1.remainder.size());

    const auto result2 = diffTiles(tileIDs(b), a);
    EXPECT_EQ(0, result2.added.size());
    EXPECT_EQ(0, result2.removed.size());
    EXPECT_EQ(4, result2.remainder.size());
}

// When listed items which have not been added or removed, the item reference is from the new set
TEST(TileDiff, New) {
    const auto a = diffTest.renderTilesByIndex(std::vector<int>{3});
    const auto b = diffTest.renderTilesByIndex(std::vector<int>{4});

    const auto result = diffTiles(tileIDs(a), b);
    EXPECT_EQ(0, result.added.size());
    EXPECT_EQ(0, result.removed.size());
    EXPECT_EQ(1, result.remainder.size());
    EXPECT_EQ(&result.remainder[0].get(), &diffTest.renderTiles[4]);
}
