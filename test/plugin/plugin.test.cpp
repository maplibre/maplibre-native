#include <gtest/gtest.h>

#include <mbgl/plugin/plugin_api.h>
#include <mbgl/plugin/plugin_registry.hpp>
#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/layers/fill_extrusion_layer.hpp>
#include <mbgl/style/rapidjson_conversion.hpp>
#include <mbgl/style/style_impl.hpp>
#include <mbgl/test/stub_file_source.hpp>
#include <mbgl/test/stub_layer_observer.hpp>
#include <mbgl/util/run_loop.hpp>

#include <algorithm>
#include <array>
#include <string>

using namespace mbgl;
using namespace mbgl::style;

namespace {

mln_plugin_string cString(const char* value) {
    return {value, std::char_traits<char>::length(value)};
}

mln_plugin_status createInstance(const mln_plugin_host_api_v1*, mln_plugin_string, void** instance) {
    *instance = reinterpret_cast<void*>(1);
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status callback(void*, const mln_plugin_frame_context_v1*) {
    return MLN_PLUGIN_STATUS_OK;
}

void destroyInstance(void*) {}

mln_plugin_status createLayout(const mln_plugin_layout_context_v1*, void** instance) {
    static int layout;
    *instance = &layout;
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status layoutFeature(void*, const mln_plugin_feature_v1*) {
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status finishLayout(void*, mln_plugin_bucket_v1* bucket) {
    bucket->query_radius = 0.0f;
    return MLN_PLUGIN_STATUS_OK;
}

void destroyLayout(void*) {}

struct Descriptor {
    explicit Descriptor(const char* id_, const char* propertyName_)
        : propertyName(propertyName_),
          id(id_) {
        defaultValue.struct_size = sizeof(defaultValue);
        defaultValue.type = MLN_PLUGIN_VALUE_BOOLEAN;
        defaultValue.data.boolean_value = 0;
        property = {sizeof(property),
                    cString(propertyName),
                    MLN_PLUGIN_VALUE_BOOLEAN,
                    MLN_PLUGIN_PROPERTY_PAINT,
                    defaultValue,
                    0,
                    0};
        extension = {sizeof(extension),
                     cString("fill-extrusion"),
                     10,
                     MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_VULKAN,
                     &property,
                     1,
                     createInstance,
                     destroyInstance,
                     callback,
                     callback,
                     nullptr};
        descriptor = {sizeof(descriptor),
                      MLN_PLUGIN_ABI_VERSION_1,
                      cString(id),
                      cString("1.0.0"),
                      MLN_PLUGIN_ABI_VERSION_1,
                      MLN_PLUGIN_ABI_VERSION_1,
                      &extension,
                      1,
                      nullptr,
                      0};
    }

    const char* propertyName;
    const char* id;
    mln_plugin_value defaultValue{};
    mln_plugin_property_descriptor_v1 property{};
    mln_plugin_layer_extension_v1 extension{};
    mln_plugin_descriptor_v1 descriptor{};
};

struct LayerTypeDescriptor {
    LayerTypeDescriptor() {
        defaultURI.struct_size = sizeof(defaultURI);
        defaultURI.type = MLN_PLUGIN_VALUE_STRING;
        defaultURI.data.string_value = {"", 0};
        property = {sizeof(property),
                    cString("test-model-uri"),
                    MLN_PLUGIN_VALUE_STRING,
                    MLN_PLUGIN_PROPERTY_LAYOUT,
                    defaultURI,
                    0,
                    0};
        properties = {property};
        shaderSource = {sizeof(shaderSource),
                        MLN_PLUGIN_BACKEND_METAL,
                        cString("shader source"),
                        {},
                        cString("testVertex"),
                        cString("testFragment")};
        shaderAttribute = {sizeof(shaderAttribute),
                           0,
                           0,
                           cString("a_position"),
                           MLN_PLUGIN_VERTEX_FLOAT_X2};
        shader = {sizeof(shader), cString("test"), &shaderSource, 1, &shaderAttribute, 1};
        layerType.struct_size = sizeof(layerType);
        layerType.layer_type = cString("test-plugin-layer");
        layerType.backend_mask = MLN_PLUGIN_BACKEND_METAL;
        layerType.render_stage = MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT;
        layerType.requires_3d = 1;
        layerType.properties = properties.data();
        layerType.property_count = properties.size();
        layerType.source_kind = MLN_PLUGIN_SOURCE_GEOMETRY;
        layerType.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POINT;
        layerType.shaders = &shader;
        layerType.shader_count = 1;
        layerType.create_layout = createLayout;
        layerType.layout_feature = layoutFeature;
        layerType.finish_layout = finishLayout;
        layerType.destroy_layout = destroyLayout;
        descriptor = {sizeof(descriptor),
                      MLN_PLUGIN_ABI_VERSION_1,
                      cString("org.maplibre.test.layer-type"),
                      cString("1.0.0"),
                      MLN_PLUGIN_ABI_VERSION_1,
                      MLN_PLUGIN_ABI_VERSION_1,
                      nullptr,
                      0,
                      &layerType,
                      1};
    }

    mln_plugin_value defaultURI{};
    mln_plugin_property_descriptor_v1 property{};
    std::array<mln_plugin_property_descriptor_v1, 1> properties{};
    mln_plugin_shader_source_v1 shaderSource{};
    mln_plugin_shader_attribute_v1 shaderAttribute{};
    mln_plugin_shader_descriptor_v1 shader{};
    mln_plugin_layer_type_v1 layerType{};
    mln_plugin_descriptor_v1 descriptor{};
};

} // namespace

TEST(PluginRegistry, RegistersAndAcceptsIdenticalRepeat) {
    Descriptor plugin("org.maplibre.test.registry-repeat", "test-registry-repeat");
    std::array<char, 256> error{};
    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&plugin.descriptor, error.data(), error.size()));
    EXPECT_EQ(MLN_PLUGIN_STATUS_ALREADY_REGISTERED,
              mln_plugin_register_v1(&plugin.descriptor, error.data(), error.size()));
    EXPECT_TRUE(mln_plugin_is_registered_v1("org.maplibre.test.registry-repeat"));
}

TEST(PluginRegistry, RejectsAbiMismatchAndConflictingProperty) {
    Descriptor badABI("org.maplibre.test.bad-abi", "test-bad-abi");
    badABI.descriptor.maximum_host_abi = 0;
    EXPECT_EQ(MLN_PLUGIN_STATUS_UNSUPPORTED_ABI, mln_plugin_register_v1(&badABI.descriptor, nullptr, 0));

    Descriptor first("org.maplibre.test.conflict-a", "test-conflicting-property");
    Descriptor second("org.maplibre.test.conflict-b", "test-conflicting-property");
    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&first.descriptor, nullptr, 0));
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&second.descriptor, nullptr, 0));
}

TEST(PluginRegistry, RejectsMalformedDuplicateAndNonPaintDescriptors) {
    Descriptor missingDestroy("org.maplibre.test.missing-destroy", "test-missing-destroy");
    missingDestroy.extension.destroy_instance = nullptr;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&missingDestroy.descriptor, nullptr, 0));

    Descriptor layout("org.maplibre.test.layout", "test-layout");
    layout.property.scope = MLN_PLUGIN_PROPERTY_LAYOUT;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&layout.descriptor, nullptr, 0));

    Descriptor duplicate("org.maplibre.test.duplicate", "test-duplicate");
    std::array<mln_plugin_property_descriptor_v1, 2> properties{duplicate.property, duplicate.property};
    duplicate.extension.properties = properties.data();
    duplicate.extension.property_count = properties.size();
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&duplicate.descriptor, nullptr, 0));
}

TEST(PluginRegistry, OrdersExtensionsByPriorityThenID) {
    Descriptor later("org.maplibre.test.priority-later", "test-priority-later");
    Descriptor earlier("org.maplibre.test.priority-earlier", "test-priority-earlier");
    later.extension.render_priority = 1000;
    earlier.extension.render_priority = -1000;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&later.descriptor, nullptr, 0));
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&earlier.descriptor, nullptr, 0));

    const auto extensions = plugin::PluginRegistry::get().extensionsForLayer("fill-extrusion");
    const auto earlierIt = std::find_if(extensions.begin(), extensions.end(), [](const auto& extension) {
        return extension.pluginID == "org.maplibre.test.priority-earlier";
    });
    const auto laterIt = std::find_if(extensions.begin(), extensions.end(), [](const auto& extension) {
        return extension.pluginID == "org.maplibre.test.priority-later";
    });
    ASSERT_NE(extensions.end(), earlierIt);
    ASSERT_NE(extensions.end(), laterIt);
    EXPECT_LT(earlierIt, laterIt);
}

TEST(PluginRegistry, RegistersSourceBoundLayerTypeAndScopedProperties) {
    LayerTypeDescriptor custom;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&custom.descriptor, nullptr, 0));
    const auto registration = plugin::PluginRegistry::get().findLayerType("test-plugin-layer");
    ASSERT_TRUE(registration);
    EXPECT_TRUE(registration->requires3D);

    JSDocument missingSource;
    missingSource.Parse("{}");
    const JSValue* missingSourceValue = &missingSource;
    conversion::Error error;
    EXPECT_FALSE(LayerManager::get()->createLayer(
        "test-plugin-layer", "missing-source", conversion::Convertible(missingSourceValue), error));
    EXPECT_NE(std::string::npos, error.message.find("requires a source"));

    JSDocument input;
    input.Parse(R"JSON({"source":"points"})JSON");
    const JSValue* inputValue = &input;
    auto layer =
        LayerManager::get()->createLayer("test-plugin-layer", "model", conversion::Convertible(inputValue), error);
    ASSERT_TRUE(layer) << error.message;
    EXPECT_STREQ("test-plugin-layer", layer->getTypeInfo()->type);
    EXPECT_EQ("points", layer->getSourceID());
    auto renderLayer = LayerManager::get()->createRenderLayer(layer->baseImpl);
    ASSERT_TRUE(renderLayer);
    EXPECT_TRUE(renderLayer->needsRendering());

    const JSValue uri("https://example.test/model.glb");
    EXPECT_FALSE(layer->setProperty("test-model-uri", conversion::Convertible(&uri), Layer::PropertyScope::Layout));
    EXPECT_TRUE(layer->setProperty("test-model-uri", conversion::Convertible(&uri), Layer::PropertyScope::Paint));
    const auto serialized = layer->serialize();
    EXPECT_EQ("https://example.test/model.glb",
              *serialized.getObject()->at("layout").getObject()->at("test-model-uri").getString());

    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};
    style.loadJSON(R"JSON({
      "version": 8,
      "sources": {
        "points": {
          "type": "geojson",
          "data": {"type":"FeatureCollection","features":[]}
        }
      },
      "layers": [{
        "id": "model-from-json",
        "type": "test-plugin-layer",
        "source": "points",
        "layout": {
          "test-model-uri": "https://example.test/from-style.glb"
        }
      }]
    })JSON");
    const auto* parsedLayer = style.getLayer("model-from-json");
    ASSERT_NE(nullptr, parsedLayer);
    EXPECT_EQ("https://example.test/from-style.glb",
              *parsedLayer->serialize().getObject()->at("layout").getObject()->at("test-model-uri").getString());
}

TEST(PluginStyleProperty, DefaultSetCloneAndSerialize) {
    Descriptor plugin("org.maplibre.test.style-property", "test-plugin-boolean");
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&plugin.descriptor, nullptr, 0));

    FillExtrusionLayer layer("buildings", "source");
    EXPECT_FALSE(layer.getPluginBoolean("test-plugin-boolean", true));

    const JSValue enabled(true);
    EXPECT_FALSE(layer.setProperty("test-plugin-boolean", conversion::Convertible(&enabled)));
    EXPECT_TRUE(layer.getPluginBoolean("test-plugin-boolean", false));

    auto clone = layer.cloneRef("buildings-copy");
    EXPECT_TRUE(clone->getPluginBoolean("test-plugin-boolean", false));

    const auto serialized = clone->serialize();
    const auto* paint = serialized.getObject()->at("paint").getObject();
    ASSERT_NE(nullptr, paint);
    EXPECT_TRUE(*paint->at("test-plugin-boolean").getBool());

    StubLayerObserver observer;
    layer.setObserver(&observer);
    int notifications = 0;
    observer.layerChanged = [&](Layer&) {
        ++notifications;
    };
    const JSValue disabled(false);
    EXPECT_FALSE(layer.setProperty("test-plugin-boolean", conversion::Convertible(&disabled)));
    EXPECT_EQ(1, notifications);
    EXPECT_FALSE(layer.getPluginBoolean("test-plugin-boolean", true));
    EXPECT_FALSE(layer.setProperty("test-plugin-boolean", conversion::Convertible(&disabled)));
    EXPECT_EQ(1, notifications);
    const auto disabledSerialized = static_cast<const Layer&>(layer).serialize();
    EXPECT_FALSE(*disabledSerialized.getObject()->at("paint").getObject()->at("test-plugin-boolean").getBool());

    const JSValue wrongType("true");
    EXPECT_TRUE(layer.setProperty("test-plugin-boolean", conversion::Convertible(&wrongType)));

    JSDocument expression;
    expression.Parse(R"(["get", "enabled"])");
    const JSValue* expressionValue = &expression;
    EXPECT_TRUE(layer.setProperty("test-plugin-boolean", conversion::Convertible(expressionValue)));

    const JSValue unknown(true);
    EXPECT_TRUE(layer.setProperty("test-unregistered-property", conversion::Convertible(&unknown)));
}
