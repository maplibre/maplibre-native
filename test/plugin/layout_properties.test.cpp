#include <mln/geometry/feature_index.hpp>
#include <mln/layout/plugin_layout.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/tile/geojson_tile_data.hpp>
#include <mln/tile/layout_resource_manager.hpp>
#include <gtest/gtest.h>

#include <set>
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

namespace {

// What the callbacks of the resource layer type saw.
struct ResourceRecord {
    std::vector<std::pair<std::string, std::string>> created; // URL and bytes.
    unsigned destroyed = 0;
    std::vector<void*> tileHandles;
    std::vector<size_t> tilePropertyCounts;
    std::vector<std::string> featureNames;
    std::vector<void*> featureHandles;
};
ResourceRecord* resourceRecord = nullptr;

struct Model {
    std::string bytes;
};

mln_plugin_status createResource(mln_plugin_string url, const uint8_t* data, size_t size, void** handle) {
    const std::string bytes(reinterpret_cast<const char*>(data), size);
    resourceRecord->created.emplace_back(toString(url), bytes);
    if (bytes == "broken") return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    *handle = new Model{bytes};
    return MLN_PLUGIN_STATUS_OK;
}

void destroyResource(void* handle) {
    ++resourceRecord->destroyed;
    delete static_cast<Model*>(handle);
}

mln_plugin_status createResourceLayout(const mln_plugin_layout_context_v1* context, void** instance) {
    resourceRecord->tilePropertyCounts.push_back(context->property_count);
    const auto* model = find(context->properties, context->property_count, "test-model");
    EXPECT_TRUE(model);
    if (!model) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    EXPECT_EQ(MLN_PLUGIN_VALUE_RESOURCE, model->value.type);
    resourceRecord->tileHandles.push_back(model->value.data.resource_value.handle);
    *instance = &layoutInstance;
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status layOutResourceFeature(void*, const mln_plugin_feature_v1* input) {
    const auto* model = find(input->layout_properties, input->layout_property_count, "test-model");
    EXPECT_TRUE(model);
    if (!model) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    // Callbacks never see the RESOURCES property itself.
    EXPECT_FALSE(find(input->layout_properties, input->layout_property_count, "test-models"));
    resourceRecord->featureNames.push_back(toString(model->value.data.resource_value.name));
    resourceRecord->featureHandles.push_back(model->value.data.resource_value.handle);
    return MLN_PLUGIN_STATUS_OK;
}

plugin::LayerType resourceLayerType() {
    plugin::LayerType registration;
    registration.pluginID = "test.layout-resources";
    registration.type = "test.layout-resources.layer";
    registration.geometryTypeMask = MLN_PLUGIN_GEOMETRY_POINT;
    registration.createLayout = createResourceLayout;
    registration.layoutFeature = layOutResourceFeature;
    registration.finishLayout = finish;
    registration.destroyLayout = destroy;
    registration.createResource = createResource;
    registration.destroyResource = destroyResource;

    plugin::PropertyDefinition models;
    models.name = "test-models";
    models.type = MLN_PLUGIN_VALUE_RESOURCES;
    models.defaultValue = std::string();
    models.isLayout = true;
    registration.properties.push_back(models);

    plugin::PropertyDefinition model;
    model.name = "test-model";
    model.type = MLN_PLUGIN_VALUE_RESOURCE;
    model.defaultValue = std::string();
    model.expressionCapabilities = MLN_PLUGIN_EXPRESSION_FEATURE;
    model.isLayout = true;
    model.resources = "test-models";
    registration.properties.push_back(model);
    return registration;
}

struct ResourceLayout {
    explicit ResourceLayout(const char* modelJSON,
                            std::vector<const char*> kinds = {"car", "truck", "bus", ""},
                            plugin::RegisteredLayerPtr layerType = nullptr)
        : registration(layerType ? std::move(layerType)
                                 : std::make_shared<const plugin::RegisteredLayer>(resourceLayerType())),
          features(std::make_shared<mapbox::feature::feature_collection<int16_t>>()) {
        auto impl = makeMutable<style::PluginStyleLayer::Impl>("test-layer", "points", registration);
        impl->pluginResources["test-models"] = {{"car", "asset://car.glb"},
                                                {"truck", "asset://truck.glb"},
                                                {"bus", "asset://bus.glb"},
                                                {"van", "asset://van.glb"}};
        impl->pluginProperties.emplace("test-model", parse(*registration->findProperty("test-model"), modelJSON));
        for (const auto* kind : kinds) {
            mapbox::feature::feature<int16_t> point{mapbox::geometry::point<int16_t>(10, 20)};
            if (*kind) point.properties = {{"kind", std::string(kind)}};
            features->push_back(std::move(point));
        }
        std::vector<Immutable<style::LayerProperties>> layers = {
            makeMutable<style::PluginStyleLayerProperties>(std::move(impl))};
        layout = std::make_unique<PluginLayout>(parameters,
                                                std::move(layers),
                                                std::make_unique<GeoJSONTileLayer>(features),
                                                registration,
                                                &layoutParameters);
    }

    void build() {
        auto featureIndex = std::make_unique<FeatureIndex>(std::make_unique<GeoJSONTileData>(*features));
        mln::unordered_map<std::string, LayerRenderData> renderData;
        layout->createBucket({}, featureIndex, renderData, false, false, parameters.tileID.canonical);
    }

    plugin::RegisteredLayerPtr registration;
    std::shared_ptr<mapbox::feature::feature_collection<int16_t>> features;
    const BucketParameters parameters{OverscaledTileID{4, 0, {4, 0, 0}}, MapMode::Static, 1, &registration->info};
    GlyphDependencies glyphDependencies;
    ImageDependencies imageDependencies;
    std::set<std::string> availableImages;
    LayoutResourceDependencies resourceDependencies;
    LayoutParameters layoutParameters{.bucketParameters = parameters,
                                      .fontFaces = std::make_shared<FontFaces>(),
                                      .glyphDependencies = glyphDependencies,
                                      .imageDependencies = imageDependencies,
                                      .availableImages = availableImages,
                                      .resourceDependencies = resourceDependencies};
    std::unique_ptr<PluginLayout> layout;
};

std::shared_ptr<const LayoutResource> loaded(const char* bytes) {
    return std::make_shared<const LayoutResource>(bytes ? std::make_shared<const std::string>(bytes) : nullptr);
}

} // namespace

TEST(PluginLayoutResources, DependsOnTheResourcesThatItsFeaturesName) {
    ResourceRecord record;
    resourceRecord = &record;
    {
        ResourceLayout tile(R"(["get", "kind"])");
        // "van" is in the layer's resources but no feature names it; the last feature names none.
        EXPECT_EQ((LayoutResourceDependencies{"asset://bus.glb", "asset://car.glb", "asset://truck.glb"}),
                  tile.resourceDependencies);
        EXPECT_TRUE(tile.layout->hasDependencies());

        LayoutResourceMap resources{{"asset://car.glb", loaded("car bytes")},
                                    {"asset://truck.glb", loaded(nullptr)},
                                    {"asset://bus.glb", loaded("broken")}};
        tile.layout->prepareResources(resources);
        // Made once per loaded resource; nothing is made from a failed load.
        EXPECT_EQ((std::vector<std::pair<std::string, std::string>>{{"asset://bus.glb", "broken"},
                                                                    {"asset://car.glb", "car bytes"}}),
                  record.created);

        tile.build();
        // create_layout gets no handle for a feature-dependent value, and no RESOURCES property.
        EXPECT_EQ((std::vector<void*>{nullptr}), record.tileHandles);
        EXPECT_EQ((std::vector<size_t>{1}), record.tilePropertyCounts);
        EXPECT_EQ((std::vector<std::string>{"car", "truck", "bus", ""}), record.featureNames);
        ASSERT_EQ(4u, record.featureHandles.size());
        ASSERT_NE(nullptr, record.featureHandles[0]);
        EXPECT_EQ("car bytes", static_cast<Model*>(record.featureHandles[0])->bytes);
        EXPECT_EQ(nullptr, record.featureHandles[1]); // Failed to load.
        EXPECT_EQ(nullptr, record.featureHandles[2]); // Failed to be created.
        EXPECT_EQ(nullptr, record.featureHandles[3]); // Names no resource.

        // Another layout of the layer type shares the handle made from a resource.
        ResourceLayout other(R"(["get", "kind"])", {"car"}, tile.registration);
        other.layout->prepareResources(resources);
        EXPECT_EQ(2u, record.created.size());
        other.build();
        EXPECT_EQ(record.featureHandles[0], record.featureHandles.back());
        EXPECT_EQ(0u, record.destroyed);
    }
    // The handle is destroyed once the last layout and resource map let the resource go.
    EXPECT_EQ(1u, record.destroyed);
    resourceRecord = nullptr;
}

TEST(PluginLayoutResources, PassesTheHandleOfAConstantResourceToCreateLayout) {
    ResourceRecord record;
    resourceRecord = &record;
    {
        ResourceLayout tile(R"("van")", {"car", "truck"});
        EXPECT_EQ((LayoutResourceDependencies{"asset://van.glb"}), tile.resourceDependencies);
        tile.layout->prepareResources({{"asset://van.glb", loaded("van bytes")}});
        tile.build();
        ASSERT_EQ(1u, record.tileHandles.size());
        ASSERT_NE(nullptr, record.tileHandles[0]);
        EXPECT_EQ("van bytes", static_cast<Model*>(record.tileHandles[0])->bytes);
        EXPECT_EQ((std::vector<void*>{record.tileHandles[0], record.tileHandles[0]}), record.featureHandles);
    }
    EXPECT_EQ(1u, record.destroyed);
    resourceRecord = nullptr;
}

TEST(PluginLayoutResources, HasNoDependenciesWhenNoResourceIsNamed) {
    ResourceRecord record;
    resourceRecord = &record;
    ResourceLayout tile(R"("plane")", {"car"});
    EXPECT_TRUE(tile.resourceDependencies.empty());
    EXPECT_FALSE(tile.layout->hasDependencies());
    tile.build();
    EXPECT_EQ((std::vector<void*>{nullptr}), record.tileHandles);
    EXPECT_TRUE(record.created.empty());
    resourceRecord = nullptr;
}
