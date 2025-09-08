#include <mbgl/test/util.hpp>

#include <mbgl/renderer/style_diff.hpp>
#include <mbgl/style/layer_impl.hpp>

using namespace mbgl;
using namespace mbgl::style;

struct TestLayerImpl : style::Layer::Impl {
    TestLayerImpl(std::string layerID, std::string sourceID)
        : Impl(std::move(layerID), std::move(sourceID)) {}

    bool hasLayoutDifference(const Layer::Impl&) const override { return false; }
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override {}

    DECLARE_LAYER_TYPE_INFO;
};

const LayerTypeInfo* TestLayerImpl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"test",
                                        LayerTypeInfo::Source::NotRequired,
                                        LayerTypeInfo::Pass3D::NotRequired,
                                        LayerTypeInfo::Layout::NotRequired,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::NotRequired};
    return &typeInfo;
}

TEST(StyleDiff, Empty) {
    auto a = makeMutable<std::vector<ImmutableLayer>>();
    auto b = makeMutable<std::vector<ImmutableLayer>>();
    auto diff = diffLayers(Immutable<std::vector<ImmutableLayer>>(std::move(a)),
                           Immutable<std::vector<ImmutableLayer>>(std::move(b)));
    ASSERT_TRUE(diff.added.empty());
    ASSERT_TRUE(diff.removed.empty());
    ASSERT_TRUE(diff.changed.empty());
}

TEST(StyleDiff, Add) {
    auto l1 = Immutable<TestLayerImpl>(makeMutable<TestLayerImpl>("id-1", "src-1"));
    auto l2 = Immutable<TestLayerImpl>(makeMutable<TestLayerImpl>("id-2", "src-2"));

    auto a = makeMutable<std::vector<ImmutableLayer>>();
    a->push_back(l1);
    auto b = makeMutable<std::vector<ImmutableLayer>>();
    b->push_back(l1);
    b->push_back(l2);

    auto diff = diffLayers(Immutable<std::vector<ImmutableLayer>>(std::move(a)),
                           Immutable<std::vector<ImmutableLayer>>(std::move(b)));

    ASSERT_EQ(1, diff.added.size());
    ASSERT_EQ(0, diff.removed.size());
    ASSERT_EQ(0, diff.changed.size());
}

TEST(StyleDiff, Remove) {
    auto l1 = Immutable<TestLayerImpl>(makeMutable<TestLayerImpl>("id-1", "src-1"));
    auto l2 = Immutable<TestLayerImpl>(makeMutable<TestLayerImpl>("id-2", "src-2"));

    auto a = makeMutable<std::vector<ImmutableLayer>>();
    a->push_back(l1);
    a->push_back(l2);
    auto b = makeMutable<std::vector<ImmutableLayer>>();
    b->push_back(l1);

    auto diff = diffLayers(Immutable<std::vector<ImmutableLayer>>(std::move(a)),
                           Immutable<std::vector<ImmutableLayer>>(std::move(b)));

    ASSERT_EQ(0, diff.added.size());
    ASSERT_EQ(1, diff.removed.size());
    ASSERT_EQ(0, diff.changed.size());
}

// Different identical instances show up as changed
TEST(StyleDiff, Compare) {
    auto a = makeMutable<std::vector<ImmutableLayer>>();
    a->push_back(makeMutable<TestLayerImpl>("id-1", "src-1"));
    a->push_back(makeMutable<TestLayerImpl>("id-2", "src-2"));

    auto b = makeMutable<std::vector<ImmutableLayer>>();
    b->push_back(makeMutable<TestLayerImpl>("id-1", "src-1"));
    b->push_back(makeMutable<TestLayerImpl>("id-2", "src-2"));

    auto diff = diffLayers(Immutable<std::vector<ImmutableLayer>>(std::move(a)),
                           Immutable<std::vector<ImmutableLayer>>(std::move(b)));

    ASSERT_EQ(0, diff.added.size());
    ASSERT_EQ(0, diff.removed.size());
    ASSERT_EQ(2, diff.changed.size());
}

// Order change shows up as add and remove
TEST(StyleDiff, Order) {
    auto l1 = Immutable<TestLayerImpl>(makeMutable<TestLayerImpl>("id-1", "src-1"));
    auto l2 = Immutable<TestLayerImpl>(makeMutable<TestLayerImpl>("id-2", "src-2"));

    auto a = makeMutable<std::vector<ImmutableLayer>>();
    a->push_back(l1);
    a->push_back(l2);

    auto b = makeMutable<std::vector<ImmutableLayer>>();
    b->push_back(l2);
    b->push_back(l1);

    auto diff = diffLayers(Immutable<std::vector<ImmutableLayer>>(std::move(a)),
                           Immutable<std::vector<ImmutableLayer>>(std::move(b)));

    ASSERT_EQ(1, diff.added.size());
    ASSERT_EQ(1, diff.removed.size());
    ASSERT_EQ(0, diff.changed.size());
}
