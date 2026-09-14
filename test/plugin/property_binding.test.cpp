#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/plugin/plugin_performance.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/tile/geojson_tile_data.hpp>
#include <gtest/gtest.h>

using namespace mln;

TEST(PluginPerformance, OptInScopesStopOnce) {
    using namespace plugin::performance;
    EXPECT_FALSE(enabled.load());
    const auto before = read();
    {
        Scope disabled(plugin::performance::Layout);
        EXPECT_FALSE(disabled.isActive());
    }
    EXPECT_EQ(before.nanoseconds, read().nanoseconds);
    enabled.store(true);
    {
        Scope active(plugin::performance::Layout);
        EXPECT_TRUE(active.isActive());
        active.stop();
        EXPECT_FALSE(active.isActive());
        const auto stopped = read();
        active.stop();
        EXPECT_EQ(stopped.nanoseconds, read().nanoseconds);
    }
    enabled.store(false);
}

namespace {
GeoJSONTileLayer source() {
    auto features = std::make_shared<mapbox::feature::feature_collection<int16_t>>();
    mapbox::feature::feature<int16_t> feature{mapbox::geometry::point<int16_t>(10, 20)};
    feature.id = uint64_t(1);
    feature.properties = {{"small", 10.0}, {"large", 30.0}, {"anchor", std::string("viewport")}};
    features->push_back(std::move(feature));
    return GeoJSONTileLayer(features);
}

style::PluginPropertyValue expression(const plugin::PropertyDefinition& definition, const char* json) {
    JSDocument document;
    document.Parse(json);
    const JSValue* value = &document;
    style::conversion::Error error;
    auto parsed = style::convertPluginPropertyValue(definition, style::conversion::Convertible(value), error);
    EXPECT_TRUE(parsed) << error.message;
    return parsed.value_or(style::defaultPluginPropertyValue(definition));
}

plugin::PropertyDefinition numberDefinition() {
    plugin::PropertyDefinition definition;
    definition.name = "test-size";
    definition.type = MLN_PLUGIN_VALUE_FLOAT;
    definition.defaultValue = 5.0;
    definition.expressionCapabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE |
                                        MLN_PLUGIN_EXPRESSION_COMPOSITE | MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
    return definition;
}
} // namespace

TEST(PluginPaintBinder, PackedCompositeEndpointsAndFeatureStateUpdates) {
    auto definition = numberDefinition();
    const auto layer = source();
    plugin::ShaderPropertyBindingDefinition binding{"test-size", MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 1, 1, 0, 4};
    auto value = expression(definition, R"(["interpolate",["linear"],["zoom"],10,["get","small"],11,["get","large"]])");
    auto snapshots = std::make_shared<const PluginFeatureData>(std::vector<PluginFeatureVertexRange>{{0, 1, 0, 4}},
                                                               layer);
    PluginPaintPropertyBinder binder(definition, binding, value, 10, {10, 0, 0}, 1, 4, snapshots);
    ASSERT_TRUE(binder.isDataDriven());
    EXPECT_EQ(gfx::AttributeDataType::Float2, binder.attributeType());
    const auto* bytes = static_cast<const float*>(binder.getVertexVector()->getRawData());
    EXPECT_FLOAT_EQ(10, bytes[0]);
    EXPECT_FLOAT_EQ(30, bytes[1]);
    EXPECT_FLOAT_EQ(0.5, binder.interpolationFactor(10.5));
    EXPECT_EQ(8u, binder.getVertexVector()->getRawSize());

    value = expression(definition, R"(["coalesce",["feature-state","size"],12])");
    EXPECT_TRUE(binder.synchronize(value)); // Refill and rebind the changed attribute data.
    FeatureStates states;
    states["1"] = {{"size", 42.0}};
    EXPECT_TRUE(binder.update(states, layer));
    bytes = static_cast<const float*>(binder.getVertexVector()->getRawData());
    EXPECT_FLOAT_EQ(42, bytes[0]);
    EXPECT_FLOAT_EQ(42, bytes[1]);
    mln_plugin_value minimum{}, maximum{};
    binder.statistics(10, minimum, maximum);
    EXPECT_FLOAT_EQ(42, maximum.data.float_value);
}

TEST(PluginPaintBinder, EnumOrdinalsAndOwnedStatisticsStrings) {
    auto definition = numberDefinition();
    definition.type = MLN_PLUGIN_VALUE_STRING;
    definition.defaultValue = std::string("map");
    definition.enumValues = {"map", "viewport"};
    const auto layer = source();
    plugin::ShaderPropertyBindingDefinition binding{
        "test-size", MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT, 0, 0, 1, 1, 0, 4};
    auto value = expression(definition, R"(["get","anchor"])");
    auto snapshots = std::make_shared<const PluginFeatureData>(std::vector<PluginFeatureVertexRange>{{0, 1, 0, 4}},
                                                               layer);
    PluginPaintPropertyBinder binder(definition, binding, value, 10, {10, 0, 0}, 1, 4, snapshots);
    const auto* bytes = static_cast<const float*>(binder.getVertexVector()->getRawData());
    EXPECT_FLOAT_EQ(1, bytes[0]);
    EXPECT_FLOAT_EQ(1, bytes[1]);
    mln_plugin_value minimum{}, maximum{};
    binder.statistics(10, minimum, maximum);
    EXPECT_EQ("viewport", std::string(maximum.data.string_value.data, maximum.data.string_value.size));
    value = expression(definition, R"("viewport")");
    EXPECT_TRUE(binder.synchronize(value));
    binder.statistics(10, minimum, maximum);
    EXPECT_EQ("viewport", std::string(maximum.data.string_value.data, maximum.data.string_value.size));
    float uniform[2]{};
    binder.writeUniform(10, 0, reinterpret_cast<uint8_t*>(uniform), sizeof(uniform));
    EXPECT_FLOAT_EQ(1, uniform[0]);
}

TEST(PluginPaintBinder, GeometryExpressionsSurvivePaintAndStateUpdates) {
    auto features = std::make_shared<mapbox::feature::feature_collection<int16_t>>();
    for (const auto coordinate : {0, 2048}) {
        mapbox::feature::feature<int16_t> feature{mapbox::geometry::point<int16_t>(coordinate, coordinate)};
        feature.id = uint64_t(features->size() + 1);
        features->push_back(std::move(feature));
    }
    // In canonical tile 1/1/1, tile coordinate 0,0 is longitude/latitude 0,0.
    const GeoJSONTileLayer layer(features);
    const auto definition = numberDefinition();
    const plugin::ShaderPropertyBindingDefinition binding{
        "test-size", MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 1, 1, 0, 4};
    const auto within = expression(definition, R"(["case",["within",{"type":"Polygon","coordinates":[
        [[-1,-1],[1,-1],[1,1],[-1,1],[-1,-1]]]}],40,4])");
    PluginPaintPropertyBinder binder(definition,
                                     binding,
                                     within,
                                     1,
                                     {1, 1, 1},
                                     1,
                                     2,
                                     std::make_shared<const PluginFeatureData>(
                                         std::vector<PluginFeatureVertexRange>{{0, 1, 0, 1}, {1, 1, 1, 1}}, layer));
    auto expectValues = [&](float inside, float outside) {
        const auto* values = static_cast<const float*>(binder.getVertexVector()->getRawData());
        EXPECT_FLOAT_EQ(inside, values[0]);
        EXPECT_FLOAT_EQ(inside, values[1]);
        EXPECT_FLOAT_EQ(outside, values[2]);
        EXPECT_FLOAT_EQ(outside, values[3]);
    };
    expectValues(40, 4);
    const auto distance = expression(definition, R"(["distance",{"type":"Point","coordinates":[0,0]}])");
    EXPECT_TRUE(binder.synchronize(distance));
    const auto* values = static_cast<const float*>(binder.getVertexVector()->getRawData());
    EXPECT_NEAR(0, values[0], 0.01);
    EXPECT_GT(values[2], 1000000);
    EXPECT_TRUE(binder.synchronize(expression(definition, R"(5)")));
    EXPECT_TRUE(binder.synchronize(within));
    expectValues(40, 4);
    EXPECT_TRUE(binder.synchronize(expression(definition, R"(["case",["within",{"type":"Polygon",
        "coordinates":[[[-1,-1],[1,-1],[1,1],[-1,1],[-1,-1]]]}],
        ["number",["feature-state","size"],40],4])")));
    EXPECT_TRUE(binder.update({{"1", {{"size", 80.0}}}, {"2", {{"size", 80.0}}}}, layer));
    expectValues(80, 4);
}

TEST(PluginPaintBinder, SnapshotsAreSharedAcrossPropertiesDrawablesAndSourceLifetime) {
    struct CountingLayer final : GeometryTileLayer {
        GeoJSONTileLayer layer = source();
        mutable size_t reads = 0;
        size_t featureCount() const override { return layer.featureCount(); }
        std::string getName() const override { return "counting"; }
        std::unique_ptr<GeometryTileFeature> getFeature(size_t i) const override {
            ++reads;
            return layer.getFeature(i);
        }
    };
    std::shared_ptr<const PluginFeatureData> snapshots;
    {
        CountingLayer layer;
        snapshots = std::make_shared<const PluginFeatureData>(
            std::vector<PluginFeatureVertexRange>{{0, 1, 0, 1}, {0, 1, 1, 1}, {0, 2, 0, 1}}, layer);
        EXPECT_EQ(1u, layer.reads);
        EXPECT_EQ(1u, snapshots->features.size());
        EXPECT_EQ(2u, snapshots->drawable(1).ranges.size());
        EXPECT_EQ((std::vector<size_t>{0, 1}), snapshots->drawable(1).byID.at("1"));
        EXPECT_EQ((std::vector<size_t>{0}), snapshots->drawable(2).byID.at("1"));
    }
    const auto definition = numberDefinition();
    const plugin::ShaderPropertyBindingDefinition binding{
        "test-size", MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 1, 1, 0, 4};
    auto makeBinder = [&](uint64_t drawable, size_t vertices) {
        return PluginPaintPropertyBinder(
            definition, binding, expression(definition, "5"), 1, {1, 0, 0}, drawable, vertices, snapshots);
    };
    auto a = makeBinder(1, 2), b = makeBinder(2, 1);
    EXPECT_EQ(3, snapshots.use_count());
    // A state change while uniform must survive a later switch to an expression.
    const auto unusedSource = source();
    EXPECT_FALSE(a.update({{"1", {{"size", 90.0}}}}, unusedSource));
    EXPECT_TRUE(a.synchronize(expression(definition, R"(["number",["feature-state","size"],12])")));
    EXPECT_TRUE(b.synchronize(expression(definition, R"(["get","small"])")));
    auto values = static_cast<const float*>(a.getVertexVector()->getRawData());
    EXPECT_FLOAT_EQ(90, values[0]);
    EXPECT_FLOAT_EQ(90, values[2]);
    EXPECT_FLOAT_EQ(10, static_cast<const float*>(b.getVertexVector()->getRawData())[0]);
    EXPECT_FALSE(a.update({{"missing", {{"size", 1.0}}}}, unusedSource));
    EXPECT_TRUE(a.update({{"1", {}}}, unusedSource)); // Removing state restores fallback.
    EXPECT_FLOAT_EQ(12, static_cast<const float*>(a.getVertexVector()->getRawData())[0]);
}

TEST(PluginPaintBinder, CachedUniformsInvalidateOnZoomAndPropertyChanges) {
    const auto layer = source();
    const auto definition = numberDefinition();
    const plugin::ShaderPropertyBindingDefinition binding{
        "test-size", MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 1, 1, 0, 4};
    PluginPaintPropertyBinder binder(
        definition,
        binding,
        expression(definition, R"(["interpolate",["linear"],["zoom"],1,10,2,20])"),
        1,
        {1, 0, 0},
        1,
        1,
        std::make_shared<const PluginFeatureData>(std::vector<PluginFeatureVertexRange>{{0, 1, 0, 1}}, layer));
    float bytes[2]{};
    auto read = [&](float zoom) {
        binder.writeUniform(zoom, 0, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes));
        return bytes[0];
    };
    EXPECT_FLOAT_EQ(10, read(1));
    EXPECT_FLOAT_EQ(10, read(1));
    EXPECT_FLOAT_EQ(15, read(1.5));
    EXPECT_FALSE(binder.synchronize(expression(definition, "30")));
    EXPECT_FLOAT_EQ(30, read(1.5));
    EXPECT_FLOAT_EQ(30, read(2));
}
