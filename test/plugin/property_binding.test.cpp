#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/tile/geojson_tile_data.hpp>
#include <gtest/gtest.h>

using namespace mln;

TEST(PluginPaintBinder, BucketRetainsImmutablePaintSnapshotAndRefreshesZoom) {
    static unsigned radiusCalls;
    radiusCalls = 0;
    plugin::LayerType registration;
    registration.queryRadius = [](const mln_plugin_property_statistics_v1*, size_t,
                                  const mln_plugin_property_value_v1*, size_t) -> float {
        ++radiusCalls;
        return 12;
    };
    PluginBucket first(registration), second(registration);
    auto snapshot = std::make_shared<const style::PluginPropertyMap>();
    EXPECT_FALSE(first.synchronizePaint("layer", snapshot, 10));
    EXPECT_FALSE(second.synchronizePaint("layer", snapshot, 10));
    EXPECT_EQ(2u, radiusCalls);
    EXPECT_EQ(snapshot, first.latestPaintProperties.at("layer"));
    EXPECT_EQ(snapshot, second.latestPaintProperties.at("layer"));
    for (unsigned frame = 0; frame < 100; ++frame) {
        EXPECT_FALSE(first.synchronizePaint("layer", snapshot, 10));
    }
    EXPECT_EQ(2u, radiusCalls);
    EXPECT_FALSE(first.synchronizePaint("layer", snapshot, 10.5));
    EXPECT_EQ(3u, radiusCalls);
    EXPECT_EQ(snapshot, first.latestPaintProperties.at("layer"));
    auto next = std::make_shared<const style::PluginPropertyMap>();
    EXPECT_FALSE(first.synchronizePaint("layer", next, 10.5));
    EXPECT_EQ(4u, radiusCalls);
    std::weak_ptr<const style::PluginPropertyMap> retained = snapshot;
    snapshot.reset();
    EXPECT_FALSE(retained.expired()); // The other tile still owns the old snapshot.
    second.synchronizePaint("layer", next, 10.5);
    EXPECT_TRUE(retained.expired());
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
    PluginPaintPropertyBinder binder(definition, binding, value, 10, 1, 4, snapshots);
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
    PluginPaintPropertyBinder binder(definition, binding, value, 10, 1, 4, snapshots);
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
    const auto& geometry = snapshots->features[0].snapshot->getGeometries();
    ASSERT_EQ(1u, geometry.size());
    ASSERT_EQ(1u, geometry[0].size());
    EXPECT_EQ(10, geometry[0][0].x);
    EXPECT_EQ(20, geometry[0][0].y);
    const auto definition = numberDefinition();
    const plugin::ShaderPropertyBindingDefinition binding{
        "test-size", MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 1, 1, 0, 4};
    auto makeBinder = [&](uint64_t drawable, size_t vertices) {
        return PluginPaintPropertyBinder(
            definition, binding, expression(definition, "5"), 1, drawable, vertices, snapshots);
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
