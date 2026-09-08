#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/tile/geojson_tile_data.hpp>
#include <gtest/gtest.h>

using namespace mln;

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
}

TEST(PluginPaintBinder, PackedCompositeEndpointsAndFeatureStateUpdates) {
    auto definition = numberDefinition();
    const auto layer = source();
    plugin::ShaderPropertyBindingDefinition binding{"test-size", MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 1, 1, 0, 4};
    auto value = expression(definition, R"(["interpolate",["linear"],["zoom"],10,["get","small"],11,["get","large"]])");
    PluginPaintPropertyBinder binder(definition, binding, value, 10, 1, 4, {{0,1,0,4}}, layer);
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
    plugin::ShaderPropertyBindingDefinition binding{"test-size", MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT, 0, 0, 1, 1, 0, 4};
    auto value = expression(definition, R"(["get","anchor"])");
    PluginPaintPropertyBinder binder(definition, binding, value, 10, 1, 4, {{0,1,0,4}}, layer);
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
