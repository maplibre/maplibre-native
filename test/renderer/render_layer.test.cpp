#include <mln/test/util.hpp>

#include <mln/renderer/buckets/debug_bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/tile_render_data.hpp>
#include <mln/style/layers/circle_layer.hpp>
#include <mln/style/layers/circle_layer_impl.hpp>
#include <mln/style/layers/circle_layer_properties.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/identity.hpp>

#include <functional>
#include <memory>
#include <vector>

using namespace mln;

namespace {

class StubTile final : public Tile {
public:
    explicit StubTile(const OverscaledTileID& id_)
        : Tile(Kind::Geometry, id_, "source") {}
    std::unique_ptr<TileRenderData> createRenderData() override { return nullptr; }
    void cancel() override {}
    bool layerPropertiesUpdated(const Immutable<style::LayerProperties>&) override { return false; }
};

Immutable<style::LayerProperties> makeLayerProperties() {
    style::CircleLayer layer("circle", "source");
    return makeMutable<style::CircleLayerProperties>(staticImmutableCast<style::CircleLayer::Impl>(layer.baseImpl));
}

class StubRenderLayer final : public RenderLayer {
public:
    StubRenderLayer()
        : RenderLayer(makeLayerProperties()) {}

    using RenderLayer::getRenderTileBucketID;
    using RenderLayer::hasRenderTile;
    using RenderLayer::setRenderTileBucketID;
    using RenderLayer::transition;
    void transition(const TransitionParameters&) override {}
    void evaluate(const PropertyEvaluationParameters&) override {}
    bool hasTransition() const override { return false; }
    bool hasCrossfade() const override { return false; }

    // Makes `tiles` the layer's cover set, as `prepare()` does with the tiles of its source.
    void showTiles(std::initializer_list<StubTile*> tiles) {
        frameTiles.clear();
        for (auto* tile : tiles) {
            frameTiles.emplace_back(tile->id.toUnwrapped(), *tile);
        }
        renderTiles = std::make_shared<const std::vector<std::reference_wrapper<const RenderTile>>>(
            frameTiles.begin(), frameTiles.end());
        updateRenderTileIDs();
    }

private:
    std::vector<RenderTile> frameTiles;
};

} // namespace

TEST(RenderLayer, UpdateRenderTileIDs) {
    StubRenderLayer layer;
    StubTile a({1, 0, 0});
    StubTile b({1, 1, 0});
    StubTile c({1, 0, 1});
    const util::SimpleIdentity bucketA;
    const util::SimpleIdentity bucketB;

    layer.showTiles({&a, &b});
    EXPECT_TRUE(layer.hasRenderTile(a.id));
    EXPECT_TRUE(layer.hasRenderTile(b.id));
    EXPECT_FALSE(layer.hasRenderTile(c.id));
    EXPECT_EQ(util::SimpleIdentity::Empty, layer.getRenderTileBucketID(a.id));
    EXPECT_TRUE(layer.setRenderTileBucketID(a.id, bucketA));
    EXPECT_TRUE(layer.setRenderTileBucketID(b.id, bucketB));
    EXPECT_FALSE(layer.setRenderTileBucketID(c.id, bucketA));

    // The same tiles again, also in another order: every tile keeps its bucket.
    layer.showTiles({&b, &a});
    EXPECT_EQ(bucketA, layer.getRenderTileBucketID(a.id));
    EXPECT_EQ(bucketB, layer.getRenderTileBucketID(b.id));

    // As many tiles as before, but one of them is new: the tile that left is dropped, the new one has no
    // bucket yet and the tile that stayed keeps its bucket.
    layer.showTiles({&a, &c});
    EXPECT_TRUE(layer.hasRenderTile(a.id));
    EXPECT_FALSE(layer.hasRenderTile(b.id));
    EXPECT_TRUE(layer.hasRenderTile(c.id));
    EXPECT_EQ(bucketA, layer.getRenderTileBucketID(a.id));
    EXPECT_EQ(util::SimpleIdentity::Empty, layer.getRenderTileBucketID(b.id));
    EXPECT_EQ(util::SimpleIdentity::Empty, layer.getRenderTileBucketID(c.id));

    // Fewer tiles.
    layer.showTiles({&c});
    EXPECT_FALSE(layer.hasRenderTile(a.id));
    EXPECT_TRUE(layer.hasRenderTile(c.id));

    // No tiles.
    layer.showTiles({});
    EXPECT_FALSE(layer.hasRenderTile(c.id));
}
