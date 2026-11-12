#include <mln/test/util.hpp>

#include <mln/renderer/buckets/debug_bucket.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/source_state.hpp>
#include <mln/renderer/tile_render_data.hpp>
#include <mln/tile/tile.hpp>

using namespace mln;

namespace {

class StubTile final : public Tile {
public:
    explicit StubTile(const OverscaledTileID& id_)
        : Tile(Kind::Geometry, id_, "source") {}
    std::unique_ptr<TileRenderData> createRenderData() override { return nullptr; }
    void cancel() override {}
    bool layerPropertiesUpdated(const Immutable<style::LayerProperties>&) override { return false; }
    void setFeatureState(const LayerFeatureStates& states) override { applied.push_back(states); }

    std::vector<LayerFeatureStates> applied;
};

void renderFrame(SourceFeatureState& state, std::initializer_list<StubTile*> tiles) {
    std::vector<RenderTile> renderTiles;
    for (auto* tile : tiles) {
        renderTiles.emplace_back(tile->id.toUnwrapped(), *tile);
    }
    state.coalesceChanges(renderTiles);
}

} // namespace

TEST(SourceFeatureState, CoalesceChanges) {
    SourceFeatureState state;
    StubTile a({1, 0, 0});
    StubTile b({1, 1, 0});
    const std::optional<std::string> layer{"layer"};

    state.updateState(layer, "1", {{"hover", true}});
    renderFrame(state, {&a});
    ASSERT_EQ(1u, a.applied.size());
    EXPECT_EQ(Value(true), a.applied[0]["layer"]["1"]["hover"]);

    // Nothing changed: nothing is applied again.
    renderFrame(state, {&a});
    EXPECT_EQ(1u, a.applied.size());

    // A tile rendered for the first time gets the stored state.
    renderFrame(state, {&a, &b});
    ASSERT_EQ(1u, b.applied.size());
    EXPECT_EQ(Value(true), b.applied[0]["layer"]["1"]["hover"]);
    EXPECT_EQ(1u, a.applied.size());

    // A tile that is up to date gets only the change.
    state.updateState(layer, "2", {{"hover", true}});
    renderFrame(state, {&a});
    ASSERT_EQ(2u, a.applied.size());
    EXPECT_EQ(1u, a.applied[1]["layer"].size());
    EXPECT_EQ(Value(true), a.applied[1]["layer"]["2"]["hover"]);

    // A tile that was not rendered when the state changed gets the whole state.
    renderFrame(state, {&a, &b});
    EXPECT_EQ(2u, a.applied.size());
    ASSERT_EQ(2u, b.applied.size());
    EXPECT_EQ(2u, b.applied[1]["layer"].size());

    // A removal and a change in the same frame both reach the tile.
    state.removeState(layer, std::string("1"), std::nullopt);
    state.updateState(layer, "3", {{"hover", true}});
    renderFrame(state, {&a, &b});
    ASSERT_EQ(3u, a.applied.size());
    EXPECT_TRUE(a.applied[2]["layer"]["1"].empty());
    EXPECT_EQ(Value(true), a.applied[2]["layer"]["3"]["hover"]);
}
