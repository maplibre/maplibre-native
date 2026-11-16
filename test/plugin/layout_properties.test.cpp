#include <mln/geometry/feature_index.hpp>
#include <mln/layout/plugin_layout.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/tile/geojson_tile_data.hpp>
#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace mln;

namespace {

// What the layout callbacks of the test layer type saw.
struct Recorded {
    std::vector<std::pair<std::string, std::string>> tileStrings;
    std::vector<float> tileScales;
    std::vector<std::string> featureKinds;
    std::vector<float> featureScales;
    std::vector<size_t> featurePropertyCounts;
};
Recorded* recorded = nullptr;

std::string toString(const mln_plugin_string& value) {
    return value.size ? std::string(value.data, value.size) : std::string{};
}

const mln_plugin_property_value_v1* find(const mln_plugin_property_value_v1* properties,
                                         size_t count,
                                         const char* name) {
    for (size_t i = 0; i < count; ++i) {
        if (toString(properties[i].name) == name) return &properties[i];
    }
    return nullptr;
}

int layoutInstance = 0;

mln_plugin_status create(const mln_plugin_layout_context_v1* context, void** instance) {
    if (const auto* kind = find(context->properties, context->property_count, "test-kind")) {
        recorded->tileStrings.emplace_back("test-kind", toString(kind->value.data.string_value));
    }
    if (const auto* scale = find(context->properties, context->property_count, "test-scale")) {
        recorded->tileScales.push_back(scale->value.data.float_value);
    }
    *instance = &layoutInstance;
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status feature(void*, const mln_plugin_feature_v1* input) {
    recorded->featurePropertyCounts.push_back(input->layout_property_count);
    const auto* kind = find(input->layout_properties, input->layout_property_count, "test-kind");
    const auto* scale = find(input->layout_properties, input->layout_property_count, "test-scale");
    EXPECT_TRUE(kind && scale);
    if (!kind || !scale) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    EXPECT_EQ(MLN_PLUGIN_VALUE_STRING, kind->value.type);
    recorded->featureKinds.push_back(toString(kind->value.data.string_value));
    recorded->featureScales.push_back(scale->value.data.float_value);
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status finish(void*, mln_plugin_bucket_v1* output) {
    *output = {};
    output->struct_size = sizeof(*output);
    return MLN_PLUGIN_STATUS_OK;
}

void destroy(void*) {}

style::PluginPropertyValue parse(const plugin::PropertyDefinition& definition, const char* json) {
    JSDocument document;
    document.Parse(json);
    const JSValue* value = &document;
    style::conversion::Error error;
    auto parsed = style::convertPluginPropertyValue(definition, style::conversion::Convertible(value), error);
    EXPECT_TRUE(parsed) << error.message;
    return parsed.value_or(style::defaultPluginPropertyValue(definition));
}

plugin::LayerType layerType() {
    plugin::LayerType registration;
    registration.pluginID = "test.layout-properties";
    registration.type = "test.layout-properties.layer";
    registration.geometryTypeMask = MLN_PLUGIN_GEOMETRY_POINT;
    registration.createLayout = create;
    registration.layoutFeature = feature;
    registration.finishLayout = finish;
    registration.destroyLayout = destroy;

    plugin::PropertyDefinition paint;
    paint.name = "test-opacity";
    paint.type = MLN_PLUGIN_VALUE_FLOAT;
    paint.defaultValue = 1.0;
    registration.properties.push_back(paint);

    plugin::PropertyDefinition kind;
    kind.name = "test-kind";
    kind.type = MLN_PLUGIN_VALUE_STRING;
    kind.defaultValue = std::string("none");
    kind.expressionCapabilities = MLN_PLUGIN_EXPRESSION_FEATURE;
    kind.isLayout = true;
    registration.properties.push_back(kind);

    plugin::PropertyDefinition scale;
    scale.name = "test-scale";
    scale.type = MLN_PLUGIN_VALUE_FLOAT;
    scale.defaultValue = 1.0;
    scale.expressionCapabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    scale.isLayout = true;
    registration.properties.push_back(scale);
    return registration;
}

void layOut(const plugin::RegisteredLayerPtr& registration, Mutable<style::PluginStyleLayer::Impl> impl) {
    auto features = std::make_shared<mapbox::feature::feature_collection<int16_t>>();
    for (const auto* kind : {"car", "truck", ""}) {
        mapbox::feature::feature<int16_t> point{mapbox::geometry::point<int16_t>(10, 20)};
        if (*kind) point.properties = {{"kind", std::string(kind)}};
        features->push_back(std::move(point));
    }
    std::vector<Immutable<style::LayerProperties>> layers = {
        makeMutable<style::PluginStyleLayerProperties>(std::move(impl))};
    auto featureIndex = std::make_unique<FeatureIndex>(std::make_unique<GeoJSONTileData>(*features));
    const BucketParameters parameters{OverscaledTileID{4, 0, {4, 0, 0}}, MapMode::Static, 1, &registration->info};
    PluginLayout layout(parameters, std::move(layers), std::make_unique<GeoJSONTileLayer>(features), registration);
    mln::unordered_map<std::string, LayerRenderData> renderData;
    layout.createBucket({}, featureIndex, renderData, false, false, parameters.tileID.canonical);
}

} // namespace

TEST(PluginLayoutProperties, EvaluatesLayoutPropertiesForTheTileAndForEachFeature) {
    Recorded record;
    recorded = &record;
    const auto registration = std::make_shared<const plugin::RegisteredLayer>(layerType());
    auto impl = makeMutable<style::PluginStyleLayer::Impl>("test-layer", "points", registration);
    impl->pluginProperties.emplace("test-kind", parse(*registration->findProperty("test-kind"), R"(["get", "kind"])"));
    impl->pluginProperties.emplace(
        "test-scale",
        parse(*registration->findProperty("test-scale"), R"(["interpolate", ["linear"], ["zoom"], 0, 1, 8, 3])"));
    layOut(registration, std::move(impl));
    recorded = nullptr;

    // create_layout gets the values at tile zoom without a feature: a feature expression falls back to its default.
    ASSERT_EQ(1u, record.tileStrings.size());
    EXPECT_EQ("none", record.tileStrings[0].second);
    ASSERT_EQ(1u, record.tileScales.size());
    EXPECT_FLOAT_EQ(2.0f, record.tileScales[0]);

    // layout_feature gets the layout properties only, evaluated for its feature. A string property coerces the
    // missing attribute of the last feature to an empty string.
    EXPECT_EQ((std::vector<size_t>{2, 2, 2}), record.featurePropertyCounts);
    EXPECT_EQ((std::vector<std::string>{"car", "truck", ""}), record.featureKinds);
    EXPECT_EQ((std::vector<float>{2.0f, 2.0f, 2.0f}), record.featureScales);
}

TEST(PluginLayoutProperties, PassesDefaultsWhenTheStyleSetsNoLayoutProperty) {
    Recorded record;
    recorded = &record;
    const auto registration = std::make_shared<const plugin::RegisteredLayer>(layerType());
    layOut(registration, makeMutable<style::PluginStyleLayer::Impl>("test-layer", "points", registration));
    recorded = nullptr;

    EXPECT_EQ((std::vector<std::string>{"none", "none", "none"}), record.featureKinds);
    EXPECT_EQ((std::vector<float>{1.0f, 1.0f, 1.0f}), record.featureScales);
}
