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

struct Descriptor {
    explicit Descriptor(const char* id_, const char* propertyName_)
        : propertyName(propertyName_),
          id(id_) {
        defaultValue.struct_size = sizeof(defaultValue);
        defaultValue.type = MLN_PLUGIN_VALUE_BOOLEAN;
        defaultValue.data.boolean_value = 0;
        property = {
            sizeof(property), cString(propertyName), MLN_PLUGIN_VALUE_BOOLEAN, MLN_PLUGIN_PROPERTY_PAINT, defaultValue};
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
                    defaultURI};
        defaultPosition.struct_size = sizeof(defaultPosition);
        defaultPosition.type = MLN_PLUGIN_VALUE_FLOAT2;
        defaultPosition.data.float2_value = {0.0f, 0.0f};
        positionProperty = {sizeof(positionProperty),
                            cString("test-model-position"),
                            MLN_PLUGIN_VALUE_FLOAT2,
                            MLN_PLUGIN_PROPERTY_LAYOUT,
                            defaultPosition};
        properties = {property, positionProperty};
        layerType = {sizeof(layerType),
                     cString("test-plugin-layer"),
                     MLN_PLUGIN_BACKEND_METAL,
                     MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT,
                     1,
                     properties.data(),
                     properties.size(),
                     createInstance,
                     destroyInstance,
                     callback,
                     callback,
                     nullptr};
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
    mln_plugin_value defaultPosition{};
    mln_plugin_property_descriptor_v1 property{};
    mln_plugin_property_descriptor_v1 positionProperty{};
    std::array<mln_plugin_property_descriptor_v1, 2> properties{};
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

TEST(PluginRegistry, RegistersSourceLessLayerTypeAndScopedProperties) {
    LayerTypeDescriptor custom;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&custom.descriptor, nullptr, 0));
    const auto registration = plugin::PluginRegistry::get().findLayerType("test-plugin-layer");
    ASSERT_TRUE(registration);
    EXPECT_TRUE(registration->requires3D);

    JSDocument empty;
    empty.Parse("{}");
    const JSValue* emptyValue = &empty;
    conversion::Error error;
    auto layer = LayerManager::get()->createLayer(
        "test-plugin-layer", "model", conversion::Convertible(emptyValue), error);
    ASSERT_TRUE(layer) << error.message;
    EXPECT_STREQ("test-plugin-layer", layer->getTypeInfo()->type);
    auto renderLayer = LayerManager::get()->createRenderLayer(layer->baseImpl);
    ASSERT_TRUE(renderLayer);
    EXPECT_TRUE(renderLayer->needsRendering());

    const JSValue uri("https://example.test/model.glb");
    EXPECT_FALSE(layer->setProperty("test-model-uri", conversion::Convertible(&uri), Layer::PropertyScope::Layout));
    EXPECT_TRUE(layer->setProperty("test-model-uri", conversion::Convertible(&uri), Layer::PropertyScope::Paint));
    const auto serialized = layer->serialize();
    EXPECT_EQ("https://example.test/model.glb",
              *serialized.getObject()->at("layout").getObject()->at("test-model-uri").getString());

    JSDocument position;
    position.Parse("[2.2945, 48.8584]");
    const JSValue* positionValue = &position;
    EXPECT_FALSE(layer->setProperty(
        "test-model-position", conversion::Convertible(positionValue), Layer::PropertyScope::Layout));
    const auto serializedPosition = layer->serialize();
    const auto* positionArray =
        serializedPosition.getObject()->at("layout").getObject()->at("test-model-position").getArray();
    ASSERT_NE(nullptr, positionArray);
    ASSERT_EQ(2u, positionArray->size());
    EXPECT_DOUBLE_EQ(2.2945, *positionArray->at(0).getDouble());

    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>();
    Style::Impl style{fileSource, 1.0, {Scheduler::GetBackground(), {}}};
    style.loadJSON(R"JSON({
      "version": 8,
      "sources": {},
      "layers": [{
        "id": "model-from-json",
        "type": "test-plugin-layer",
        "layout": {
          "test-model-uri": "https://example.test/from-style.glb",
          "test-model-position": [2.2945, 48.8584]
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
    expression.Parse(R"(["literal", true])");
    const JSValue* expressionValue = &expression;
    EXPECT_TRUE(layer.setProperty("test-plugin-boolean", conversion::Convertible(expressionValue)));

    const JSValue unknown(true);
    EXPECT_TRUE(layer.setProperty("test-unregistered-property", conversion::Convertible(&unknown)));
}
