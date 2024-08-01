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

struct Test {
    mbgl::style::VectorSource initialized{"source", Tileset{{"tiles"}}};
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>();
    TaggedScheduler threadPool{Scheduler::GetBackground(), {}};
    FakeTileSource source{initialized.baseImpl, threadPool};
    const std::vector<UnwrappedTileID> tileIDs{
        {0, {0, 0, 0}},
        {0, {1, 0, 0}},
        {0, {1, 1, 0}},
        {1, {0, 0, 0}},
    };
    FakeTile tiles[4]{
        {source, tileIDs[0].overscaleTo(tileIDs[0].canonical.z)},
        {source, tileIDs[1].overscaleTo(tileIDs[1].canonical.z)},
        {source, tileIDs[2].overscaleTo(tileIDs[2].canonical.z)},
        {source, tileIDs[3].overscaleTo(tileIDs[3].canonical.z)},
    };

    Test() { initialized.loadDescription(*fileSource); }
    RenderTile renderTile(std::size_t i) { return {tiles[i].id.toUnwrapped(), tiles[i]}; }
    template <typename T>
    std::vector<RenderTile> renderTiles(T indexes) {
        std::vector<RenderTile> result;
        std::for_each(indexes.begin(), indexes.end(), [&](auto i) { result.push_back(renderTile(i)); });
        return result;
    }
} diffTest;

} // namespace

TEST(TileDiff, Empty) {
    std::vector<RenderTile> a, b;
    auto result = diffTiles(a.begin(), a.end(), b.begin(), b.end());
    EXPECT_EQ(0, result.added.size());
    EXPECT_EQ(0, result.removed.size());
    EXPECT_EQ(0, result.remainder.size());
}

TEST(TileDiff, Add) {
    const auto a = diffTest.renderTiles(std::vector<int>{0, 1});
    const auto b = diffTest.renderTiles(std::vector<int>{0, 1, 2, 3});

    const auto result = diffTiles(a.begin(), a.end(), b.begin(), b.end());
    EXPECT_EQ(2, result.added.size());
    EXPECT_EQ(result.added[0], diffTest.tileIDs[2]);
    EXPECT_EQ(result.added[1], diffTest.tileIDs[3]);
    EXPECT_EQ(0, result.removed.size());
    EXPECT_EQ(2, result.remainder.size());
}

TEST(TileDiff, Remove) {
    const auto a = diffTest.renderTiles(std::vector<int>{0, 1, 2, 3});
    const auto b = diffTest.renderTiles(std::vector<int>{0, 1});
    const auto result = diffTiles(a.begin(), a.end(), b.begin(), b.end());
    EXPECT_EQ(0, result.added.size());
    EXPECT_EQ(2, result.removed.size());
    EXPECT_EQ(result.removed[0], diffTest.tileIDs[2]);
    EXPECT_EQ(result.removed[1], diffTest.tileIDs[3]);
    EXPECT_EQ(2, result.remainder.size());
}

TEST(TileDiff, Equal) {
    const auto a = diffTest.renderTiles(std::vector<int>{0, 1, 2, 3});
    const auto b = diffTest.renderTiles(std::vector<int>{0, 1, 2, 3});

    const auto result1 = diffTiles(a.begin(), a.end(), a.begin(), a.end());
    EXPECT_EQ(0, result1.added.size());
    EXPECT_EQ(0, result1.removed.size());
    EXPECT_EQ(4, result1.remainder.size());

    const auto result2 = diffTiles(a.begin(), a.end(), b.begin(), b.end());
    EXPECT_EQ(0, result2.added.size());
    EXPECT_EQ(0, result2.removed.size());
    EXPECT_EQ(4, result2.remainder.size());
}
