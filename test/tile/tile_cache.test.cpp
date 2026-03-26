#include <mbgl/test/util.hpp>

#include <mbgl/tile/tile_cache.hpp>

#include <mbgl/test/fake_file_source.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>
#include <mbgl/tile/vector_tile.hpp>
#include <mbgl/tile/vector_mvt_tile_data.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/map/transform.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/query.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/test/vector_tile_test.hpp>
#include <mbgl/text/glyph_manager.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/identity.hpp>

#include <memory>

using namespace mbgl;

namespace {

class VectorTileMock : public VectorTile {
public:
    VectorTileMock(const OverscaledTileID& id_,
                   std::string sourceID_,
                   const TileParameters& parameters,
                   const Tileset& tileset)
        : VectorTile(id_, std::move(sourceID_), parameters, tileset) {
        renderable = true;
    }

    void setData(const std::shared_ptr<const std::string>&) override {}

    util::SimpleIdentity uniqueId;
};

} // namespace

TEST(TileCache, Smoke) {
    VectorTileTest test;
    {
        TileCache cache(test.threadPool, 1);
        const OverscaledTileID id(0, 0, 0);
        auto tile = std::make_unique<VectorTileMock>(id, "source", test.tileParameters, test.tileset);

        cache.add(id, std::move(tile));
        EXPECT_TRUE(cache.has(id));
        cache.clear();
        EXPECT_FALSE(cache.has(id));
    }
}

TEST(TileCache, Issue15926) {
    VectorTileTest test;
    {
        TileCache cache(test.threadPool, 2);
        const OverscaledTileID id0(0, 0, 0);
        const OverscaledTileID id1(1, 0, 0);
        auto tile1 = std::make_unique<VectorTileMock>(id0, "source", test.tileParameters, test.tileset);
        auto tile2 = std::make_unique<VectorTileMock>(id0, "source", test.tileParameters, test.tileset);
        auto tile3 = std::make_unique<VectorTileMock>(id1, "source", test.tileParameters, test.tileset);
        auto tile4 = std::make_unique<VectorTileMock>(id0, "source", test.tileParameters, test.tileset);
        const auto tile1Id = tile1->uniqueId;

        // add
        cache.add(id0, std::move(tile1));
        EXPECT_TRUE(cache.has(id0));

        // adding a key already present doesn't replace the existing item
        cache.add(id0, std::move(tile2));
        EXPECT_EQ(tile1Id, static_cast<VectorTileMock*>(cache.get(id0))->uniqueId);

        // Evict on add
        cache.setSize(1);
        cache.add(id1, std::move(tile3));
        EXPECT_FALSE(cache.has(id0));
        EXPECT_TRUE(cache.has(id1));

        // Evict due to size limit change
        cache.setSize(2);
        cache.add(id0, std::move(tile4));
        EXPECT_TRUE(cache.has(id0));
        EXPECT_TRUE(cache.has(id1));
        cache.setSize(1);
        // older item should be evicted
        EXPECT_TRUE(cache.has(id0));
        EXPECT_FALSE(cache.has(id1));
    }
}
