#include <mln/test/util.hpp>

#include <mln/renderer/group_by_layout.hpp>
#include <mln/style/conversion/filter.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/filter.hpp>
#include <mln/style/layers/line_layer.hpp>
#include <mln/style/layers/line_layer_impl.hpp>
#include <mln/style/layers/line_layer_properties.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace mln;
using namespace mln::style;

namespace {

// Builds one line layer the way the style parser does: a fresh filter (and so a fresh
// expression object) is parsed from the JSON text for every layer.
std::unique_ptr<LineLayer> makeLine(const std::string& id, const char* filterJSON, float lineWidth = 2.0f) {
    auto layer = std::make_unique<LineLayer>(id, "source");
    layer->setSourceLayer("road");
    if (filterJSON) {
        conversion::Error error;
        auto filter = conversion::convertJSON<Filter>(filterJSON, error);
        EXPECT_TRUE(filter.has_value());
        layer->setFilter(*filter);
    }
    layer->setLineWidth(PropertyValue<float>(lineWidth));
    return layer;
}

Immutable<LayerProperties> properties(const LineLayer& layer) {
    return makeMutable<LineLayerProperties>(staticImmutableCast<LineLayer::Impl>(layer.baseImpl));
}

std::size_t groupCount(const std::vector<const LineLayer*>& layers) {
    std::vector<Immutable<LayerProperties>> props;
    for (const auto* layer : layers) props.push_back(properties(*layer));
    return groupLayers(props).size();
}

const char* roadFilter = R"(["==", ["get", "class"], "street"])";

} // namespace

TEST(GroupByLayout, EqualFiltersShareAGroup) {
    // A casing and a fill layer: same filter text, separate expression objects, only
    // a paint property differs.
    auto casing = makeLine("casing", roadFilter, 6.0f);
    auto fill = makeLine("fill", roadFilter, 4.0f);

    std::vector<Immutable<LayerProperties>> props{properties(*casing), properties(*fill)};
    auto groups = groupLayers(props);

    ASSERT_EQ(1u, groups.size());
    EXPECT_EQ(2u, groups.begin()->second.size());
}

TEST(GroupByLayout, DifferentFiltersGetSeparateGroups) {
    auto street = makeLine("street", roadFilter);
    auto motorway = makeLine("motorway", R"(["==", ["get", "class"], "motorway"])");
    EXPECT_EQ(2u, groupCount({street.get(), motorway.get()}));
}

TEST(GroupByLayout, LayersWithoutFilterShareAGroup) {
    auto a = makeLine("a", nullptr);
    auto b = makeLine("b", nullptr);
    EXPECT_EQ(1u, groupCount({a.get(), b.get()}));
}

TEST(GroupByLayout, FilterAndNoFilterGetSeparateGroups) {
    auto filtered = makeLine("filtered", roadFilter);
    auto plain = makeLine("plain", nullptr);
    EXPECT_EQ(2u, groupCount({filtered.get(), plain.get()}));
}

TEST(GroupByLayout, LayoutDifferenceGetsSeparateGroups) {
    auto a = makeLine("a", roadFilter);
    auto b = makeLine("b", roadFilter);
    b->setLineCap(PropertyValue<LineCapType>(LineCapType::Round));
    EXPECT_EQ(2u, groupCount({a.get(), b.get()}));
}

TEST(GroupByLayout, ManyEqualFiltersCollapseToOneGroupPerDistinctFilter) {
    std::vector<std::unique_ptr<LineLayer>> owned;
    std::vector<const LineLayer*> layers;
    const char* filters[] = {
        R"(["==", ["get", "class"], "street"])",
        R"(["==", ["get", "class"], "motorway"])",
        R"(["==", ["get", "class"], "path"])",
    };
    for (int i = 0; i < 12; ++i) {
        owned.push_back(makeLine("layer-" + std::to_string(i), filters[i % 3]));
        layers.push_back(owned.back().get());
    }
    EXPECT_EQ(3u, groupCount(layers));
}
