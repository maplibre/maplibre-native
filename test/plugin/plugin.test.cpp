#include <gtest/gtest.h>

#include <mln/plugin/plugin_api.h>
#include <mln/plugin/plugin_registry.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/layers/fill_extrusion_layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/style/style_impl.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_layer_observer.hpp>
#include <mln/util/run_loop.hpp>

#include <algorithm>
#include <array>
#include <string>

using namespace mln;
using namespace mln::style;

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

mln_plugin_status updateUniform(const mln_plugin_uniform_context_v1*, uint32_t, uint8_t*, size_t) {
    return MLN_PLUGIN_STATUS_OK;
}

struct Descriptor {
    explicit Descriptor(const char* id_, const char* propertyName_)
        : propertyName(propertyName_),
          id(id_) {
        defaultValue.struct_size = sizeof(defaultValue);
        defaultValue.type = MLN_PLUGIN_VALUE_BOOLEAN;
        defaultValue.data.boolean_value = 0;
        property.struct_size = sizeof(property);
        property.name = cString(propertyName);
        property.type = MLN_PLUGIN_VALUE_BOOLEAN;
        property.scope = MLN_PLUGIN_PROPERTY_PAINT;
        property.default_value = defaultValue;
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
        property.struct_size = sizeof(property);
        property.name = cString("test-model-uri");
        property.type = MLN_PLUGIN_VALUE_STRING;
        property.scope = MLN_PLUGIN_PROPERTY_LAYOUT;
        property.default_value = defaultURI;
        properties = {property};
        shaderSource = {sizeof(shaderSource),
                        MLN_PLUGIN_BACKEND_METAL,
                        cString("shader source"),
                        {},
                        cString("testVertex"),
                        cString("testFragment")};
        shaderAttribute = {sizeof(shaderAttribute), 0, 0, cString("a_position"), MLN_PLUGIN_VERTEX_FLOAT_X2};
        shader.struct_size = sizeof(shader);
        shader.shader_id = cString("test");
        shader.sources = &shaderSource;
        shader.source_count = 1;
        shader.attributes = &shaderAttribute;
        shader.attribute_count = 1;
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

struct DynamicLayerTypeDescriptor {
    DynamicLayerTypeDescriptor(const char* pluginID_,
                               const char* layerType_,
                               uint32_t capabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE |
                                                       MLN_PLUGIN_EXPRESSION_COMPOSITE |
                                                       MLN_PLUGIN_EXPRESSION_FEATURE_STATE)
        : pluginID(pluginID_),
          layerTypeName(layerType_) {
        defaultValue.struct_size = sizeof(defaultValue);
        defaultValue.type = MLN_PLUGIN_VALUE_FLOAT;
        defaultValue.data.float_value = 4.0f;
        property.struct_size = sizeof(property);
        property.name = cString("test-size");
        property.type = MLN_PLUGIN_VALUE_FLOAT;
        property.scope = MLN_PLUGIN_PROPERTY_PAINT;
        property.default_value = defaultValue;
        property.expression_capabilities = capabilities;
        property.supports_transitions = 1;
        shaderSource = {sizeof(shaderSource),
                        MLN_PLUGIN_BACKEND_METAL,
                        cString("shader source"),
                        {},
                        cString("testVertex"),
                        cString("testFragment")};
        shaderAttributes = {{
            {sizeof(mln_plugin_shader_attribute_v1), 0, 0, cString("a_position"), MLN_PLUGIN_VERTEX_FLOAT_X2},
            {sizeof(mln_plugin_shader_attribute_v1), 1, 1, cString("a_size_min"), MLN_PLUGIN_VERTEX_FLOAT},
            {sizeof(mln_plugin_shader_attribute_v1), 2, 2, cString("a_size_max"), MLN_PLUGIN_VERTEX_FLOAT},
        }};
        uniform = {sizeof(uniform),
                   0,
                   cString("TestDrawableUBO"),
                   16,
                   MLN_PLUGIN_SHADER_STAGE_VERTEX,
                   MLN_PLUGIN_UNIFORM_SCOPE_DRAWABLE};
        binding = {sizeof(binding), cString("test-size"), MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 1, 2, 0, 4};
        shader.struct_size = sizeof(shader);
        shader.shader_id = cString("test-dynamic");
        shader.sources = &shaderSource;
        shader.source_count = 1;
        shader.attributes = shaderAttributes.data();
        shader.attribute_count = shaderAttributes.size();
        shader.uniform_blocks = &uniform;
        shader.uniform_block_count = 1;
        shader.property_bindings = &binding;
        shader.property_binding_count = 1;
        layerType.struct_size = sizeof(layerType);
        layerType.layer_type = cString(layerTypeName);
        layerType.backend_mask = MLN_PLUGIN_BACKEND_METAL;
        layerType.render_stage = MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT;
        layerType.properties = &property;
        layerType.property_count = 1;
        layerType.source_kind = MLN_PLUGIN_SOURCE_GEOMETRY;
        layerType.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POINT;
        layerType.shaders = &shader;
        layerType.shader_count = 1;
        layerType.create_layout = createLayout;
        layerType.layout_feature = layoutFeature;
        layerType.finish_layout = finishLayout;
        layerType.destroy_layout = destroyLayout;
        layerType.update_uniform_block = updateUniform;
        descriptor = {sizeof(descriptor),
                      MLN_PLUGIN_ABI_VERSION_1,
                      cString(pluginID),
                      cString("1.0.0"),
                      MLN_PLUGIN_ABI_VERSION_1,
                      MLN_PLUGIN_ABI_VERSION_1,
                      nullptr,
                      0,
                      &layerType,
                      1};
    }

    const char* pluginID;
    const char* layerTypeName;
    mln_plugin_value defaultValue{};
    mln_plugin_property_descriptor_v1 property{};
    mln_plugin_shader_source_v1 shaderSource{};
    std::array<mln_plugin_shader_attribute_v1, 3> shaderAttributes{};
    mln_plugin_uniform_block_descriptor_v1 uniform{};
    mln_plugin_shader_property_binding_v1 binding{};
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
    Descriptor truncatedProperty("org.maplibre.test.truncated-property", "test-truncated-property");
    truncatedProperty.property.struct_size = sizeof(truncatedProperty.property) - 1;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&truncatedProperty.descriptor, nullptr, 0));

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

TEST(PluginRegistry, RegistersRasterDEMRenderGraphAndRejectsForwardDependencies) {
    const mln_plugin_shader_source_v1 source{sizeof(mln_plugin_shader_source_v1),
                                             MLN_PLUGIN_BACKEND_METAL,
                                             cString("shader source"),
                                             {},
                                             cString("testVertex"),
                                             cString("testFragment")};
    const mln_plugin_shader_attribute_v1 attribute{
        sizeof(mln_plugin_shader_attribute_v1), 0, 0, cString("a_position"), MLN_PLUGIN_VERTEX_INT16_X2};
    const mln_plugin_uniform_block_descriptor_v1 uniform{sizeof(mln_plugin_uniform_block_descriptor_v1),
                                                         0,
                                                         cString("TestDrawableUBO"),
                                                         16,
                                                         MLN_PLUGIN_SHADER_STAGE_VERTEX,
                                                         MLN_PLUGIN_UNIFORM_SCOPE_DRAWABLE};
    const mln_plugin_shader_texture_v1 texture{sizeof(mln_plugin_shader_texture_v1), 0, 0, cString("u_image")};
    const mln_plugin_shader_descriptor_v1 shader{sizeof(mln_plugin_shader_descriptor_v1),
                                                 cString("dem-shader"),
                                                 &source,
                                                 1,
                                                 &attribute,
                                                 1,
                                                 &uniform,
                                                 1,
                                                 &texture,
                                                 1,
                                                 nullptr,
                                                 0};
    const mln_plugin_render_target_descriptor_v1 target{sizeof(mln_plugin_render_target_descriptor_v1),
                                                        1,
                                                        MLN_PLUGIN_RENDER_TARGET_SOURCE_TILE,
                                                        MLN_PLUGIN_RENDER_TARGET_RGBA8,
                                                        MLN_PLUGIN_RENDER_TARGET_PER_TILE,
                                                        1.0f,
                                                        1.0f,
                                                        {0.0f, 0.0f, 0.0f, 1.0f}};
    const mln_plugin_texture_binding_v1 prepareTexture{sizeof(mln_plugin_texture_binding_v1),
                                                       0,
                                                       MLN_PLUGIN_TEXTURE_SOURCE_RASTER_DEM,
                                                       0,
                                                       {},
                                                       MLN_PLUGIN_TEXTURE_FILTER_NEAREST,
                                                       MLN_PLUGIN_TEXTURE_WRAP_CLAMP,
                                                       MLN_PLUGIN_TEXTURE_WRAP_CLAMP};
    mln_plugin_texture_binding_v1 finalTexture{sizeof(mln_plugin_texture_binding_v1),
                                               0,
                                               MLN_PLUGIN_TEXTURE_SOURCE_RENDER_TARGET,
                                               1,
                                               {},
                                               MLN_PLUGIN_TEXTURE_FILTER_LINEAR,
                                               MLN_PLUGIN_TEXTURE_WRAP_CLAMP,
                                               MLN_PLUGIN_TEXTURE_WRAP_CLAMP};
    std::array<mln_plugin_render_pass_descriptor_v1, 2> passes{{
        {sizeof(mln_plugin_render_pass_descriptor_v1),
         1,
         cString("dem-shader"),
         MLN_PLUGIN_GRAPH_GEOMETRY_RASTER_DEM_FULL_TILE,
         1,
         MLN_PLUGIN_RENDER_STAGE_PREPARE,
         MLN_PLUGIN_DRAW_MODE_TRIANGLES,
         MLN_PLUGIN_DEPTH_DISABLED,
         MLN_PLUGIN_BLEND_REPLACE,
         0,
         0,
         MLN_PLUGIN_TILE_PROJECTION_ALIGNED,
         &prepareTexture,
         1},
        {sizeof(mln_plugin_render_pass_descriptor_v1),
         2,
         cString("dem-shader"),
         MLN_PLUGIN_GRAPH_GEOMETRY_RASTER_DEM_MASKED_TILE,
         0,
         MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT,
         MLN_PLUGIN_DRAW_MODE_TRIANGLES,
         MLN_PLUGIN_DEPTH_DISABLED,
         MLN_PLUGIN_BLEND_PREMULTIPLIED_ALPHA,
         0,
         0,
         MLN_PLUGIN_TILE_PROJECTION_ALIGNED,
         &finalTexture,
         1},
    }};
    const mln_plugin_render_graph_v1 graph{
        sizeof(mln_plugin_render_graph_v1), &target, 1, passes.data(), passes.size()};
    mln_plugin_layer_type_v1 layerType{};
    layerType.struct_size = sizeof(layerType);
    layerType.layer_type = cString("org.maplibre.test.raster-dem-graph");
    layerType.backend_mask = MLN_PLUGIN_BACKEND_METAL;
    layerType.render_stage = MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT;
    layerType.source_kind = MLN_PLUGIN_SOURCE_RASTER_DEM;
    layerType.shaders = &shader;
    layerType.shader_count = 1;
    layerType.render_graph = &graph;
    layerType.update_uniform_block = updateUniform;
    layerType.participates_in_3d_pass = 1;
    mln_plugin_descriptor_v1 descriptor{sizeof(mln_plugin_descriptor_v1),
                                        MLN_PLUGIN_ABI_VERSION_1,
                                        cString("org.maplibre.test.raster-dem-graph-plugin"),
                                        cString("1.0.0"),
                                        MLN_PLUGIN_ABI_VERSION_1,
                                        MLN_PLUGIN_ABI_VERSION_1,
                                        nullptr,
                                        0,
                                        &layerType,
                                        1};
    char errorBuffer[256]{};
    layerType.update_uniform_block = nullptr;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT,
              mln_plugin_register_v1(&descriptor, errorBuffer, sizeof(errorBuffer)));
    EXPECT_NE(std::string::npos, std::string(errorBuffer).find("uniforms without an update callback"));

    layerType.update_uniform_block = updateUniform;
    passes[0].tile_projection = static_cast<mln_plugin_tile_projection>(0);
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&descriptor, nullptr, 0));
    passes[0].tile_projection = MLN_PLUGIN_TILE_PROJECTION_ALIGNED;
    passes[0].draw_mode = MLN_PLUGIN_DRAW_MODE_LINES;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&descriptor, nullptr, 0));
    passes[0].draw_mode = MLN_PLUGIN_DRAW_MODE_TRIANGLES;
    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&descriptor, nullptr, 0));
    const auto registration = plugin::PluginRegistry::get().findLayerType("org.maplibre.test.raster-dem-graph");
    ASSERT_TRUE(registration);
    ASSERT_TRUE(registration->renderGraph);
    EXPECT_TRUE(registration->participatesIn3DPass);
    EXPECT_EQ(2u, registration->renderGraph->passes.size());
    EXPECT_EQ(MLN_PLUGIN_TILE_PROJECTION_ALIGNED, registration->renderGraph->passes.front().tileProjection);

    finalTexture.render_target_id = 9;
    layerType.layer_type = cString("org.maplibre.test.raster-dem-forward-reference");
    descriptor.plugin_id = cString("org.maplibre.test.raster-dem-forward-reference-plugin");
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&descriptor, nullptr, 0));
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
    EXPECT_TRUE(registration->participatesIn3DPass);

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
    auto layer = LayerManager::get()->createLayer(
        "test-plugin-layer", "model", conversion::Convertible(inputValue), error);
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

TEST(PluginRegistry, ValidatesDynamicShaderPropertyBindings) {
    DynamicLayerTypeDescriptor valid("org.maplibre.test.dynamic-valid", "test-dynamic-valid");
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&valid.descriptor, nullptr, 0));
    const auto registration = plugin::PluginRegistry::get().findLayerType("test-dynamic-valid");
    ASSERT_TRUE(registration);
    ASSERT_EQ(1u, registration->shaders.size());
    ASSERT_EQ(1u, registration->shaders.front().propertyBindings.size());
    EXPECT_EQ("test-size", registration->shaders.front().propertyBindings.front().propertyName);

    DynamicLayerTypeDescriptor overlap("org.maplibre.test.dynamic-overlap", "test-dynamic-overlap");
    overlap.binding.interpolation_uniform_byte_offset = 0;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&overlap.descriptor, nullptr, 0));

    DynamicLayerTypeDescriptor unaligned("org.maplibre.test.dynamic-unaligned", "test-dynamic-unaligned");
    unaligned.binding.uniform_byte_offset = 2;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&unaligned.descriptor, nullptr, 0));

    DynamicLayerTypeDescriptor missingBinding("org.maplibre.test.dynamic-unbound", "test-dynamic-unbound");
    missingBinding.shader.property_bindings = nullptr;
    missingBinding.shader.property_binding_count = 0;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&missingBinding.descriptor, nullptr, 0));

    DynamicLayerTypeDescriptor wrongProperty("org.maplibre.test.dynamic-wrong-property", "test-dynamic-wrong-property");
    wrongProperty.binding.property_name = cString("not-a-property");
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&wrongProperty.descriptor, nullptr, 0));

    Descriptor booleanTransition("org.maplibre.test.boolean-transition", "test-boolean-transition");
    booleanTransition.property.supports_transitions = 1;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&booleanTransition.descriptor, nullptr, 0));
}

TEST(PluginStyleProperty, EnforcesExpressionDependencyCapabilities) {
    DynamicLayerTypeDescriptor cameraOnly(
        "org.maplibre.test.dynamic-camera", "test-dynamic-camera", MLN_PLUGIN_EXPRESSION_CAMERA);
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&cameraOnly.descriptor, nullptr, 0));
    JSDocument layerJSON;
    layerJSON.Parse(R"JSON({"source":"points"})JSON");
    const JSValue* layerValue = &layerJSON;
    conversion::Error createError;
    auto layer = LayerManager::get()->createLayer(
        "test-dynamic-camera", "dynamic", conversion::Convertible(layerValue), createError);
    ASSERT_TRUE(layer) << createError.message;

    JSDocument camera;
    camera.Parse(R"JSON(["interpolate",["linear"],["zoom"],0,2,20,20])JSON");
    const JSValue* cameraValue = &camera;
    EXPECT_FALSE(layer->setProperty("test-size", conversion::Convertible(cameraValue)));

    JSDocument feature;
    feature.Parse(R"JSON(["get","size"])JSON");
    const JSValue* featureValue = &feature;
    EXPECT_TRUE(layer->setProperty("test-size", conversion::Convertible(featureValue)));

    JSDocument composite;
    composite.Parse(R"JSON(["interpolate",["linear"],["zoom"],0,["get","small"],20,["get","large"]])JSON");
    const JSValue* compositeValue = &composite;
    EXPECT_TRUE(layer->setProperty("test-size", conversion::Convertible(compositeValue)));

    JSDocument featureState;
    featureState.Parse(R"JSON(["coalesce",["feature-state","size"],4])JSON");
    const JSValue* featureStateValue = &featureState;
    EXPECT_TRUE(layer->setProperty("test-size", conversion::Convertible(featureStateValue)));
}

TEST(PluginStyleProperty, StoresSerializesAndInterpolatesTransitions) {
    DynamicLayerTypeDescriptor dynamic("org.maplibre.test.dynamic-transition", "test-dynamic-transition");
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&dynamic.descriptor, nullptr, 0));
    JSDocument layerJSON;
    layerJSON.Parse(R"JSON({"source":"points"})JSON");
    const JSValue* layerValue = &layerJSON;
    conversion::Error createError;
    auto layer = LayerManager::get()->createLayer(
        "test-dynamic-transition", "dynamic-transition", conversion::Convertible(layerValue), createError);
    ASSERT_TRUE(layer) << createError.message;

    JSDocument transitionJSON;
    transitionJSON.Parse(R"JSON({"duration":100,"delay":20})JSON");
    const JSValue* transitionValue = &transitionJSON;
    EXPECT_FALSE(layer->setProperty(
        "test-size-transition", conversion::Convertible(transitionValue), Layer::PropertyScope::Paint));
    EXPECT_TRUE(layer->setProperty(
        "test-size-transition", conversion::Convertible(transitionValue), Layer::PropertyScope::Layout));
    const auto serialized = layer->serialize();
    const auto* paint = serialized.getObject()->at("paint").getObject();
    ASSERT_NE(nullptr, paint);
    const auto* transition = paint->at("test-size-transition").getObject();
    ASSERT_NE(nullptr, transition);
    EXPECT_EQ(100, *transition->at("duration").getInt());
    EXPECT_EQ(20, *transition->at("delay").getInt());

    plugin::PropertyDefinition definition{"test",
                                          "test-dynamic-transition",
                                          "test-size",
                                          MLN_PLUGIN_VALUE_FLOAT,
                                          MLN_PLUGIN_PROPERTY_PAINT,
                                          Value{4.0},
                                          MLN_PLUGIN_EXPRESSION_CAMERA,
                                          true,
                                          false,
                                          {},
                                          {},
                                          0,
                                          {}};
    style::PluginPropertyValue from{style::PluginPropertyValue::TypedValue{PropertyValue<float>{0.0f}}};
    style::PluginPropertyValue to{style::PluginPropertyValue::TypedValue{PropertyValue<float>{10.0f}}};
    const auto start = Clock::now();
    style::PluginTransitioningPropertyValue transitioning{
        std::move(to),
        style::PluginTransitioningPropertyValue{std::move(from)},
        TransitionOptions{std::chrono::milliseconds(100), Duration::zero()},
        start};
    const auto midpoint = transitioning.evaluate(0.0f, definition, start + std::chrono::milliseconds(50));
    const auto midpointStyle = midpoint.toStyleProperty();
    const auto value = numericValue<double>(midpointStyle.getValue());
    ASSERT_TRUE(value);
    EXPECT_GT(*value, 0.0);
    EXPECT_LT(*value, 10.0);
    EXPECT_TRUE(transitioning.hasTransition());
    const auto completed = transitioning.evaluate(0.0f, definition, start + std::chrono::milliseconds(100));
    EXPECT_DOUBLE_EQ(10.0, *numericValue<double>(completed.toStyleProperty().getValue()));
    EXPECT_FALSE(transitioning.hasTransition());
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

TEST(PluginStyleProperty, EnforcesArrayNumericAndEnumConstraints) {
    Descriptor numeric("org.maplibre.test.constrained-array", "test-constrained-array");
    const std::array<float, 1> defaultArray{45.0f};
    numeric.defaultValue.type = MLN_PLUGIN_VALUE_FLOAT_ARRAY;
    numeric.defaultValue.data.float_array_value = {defaultArray.data(), defaultArray.size()};
    numeric.property.type = MLN_PLUGIN_VALUE_FLOAT_ARRAY;
    numeric.property.default_value = numeric.defaultValue;
    numeric.property.accepts_scalar = 1;
    numeric.property.has_minimum = 1;
    numeric.property.minimum = 0.0f;
    numeric.property.has_maximum = 1;
    numeric.property.maximum = 90.0f;
    numeric.property.maximum_array_length = 4;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&numeric.descriptor, nullptr, 0));

    FillExtrusionLayer layer("constrained", "source");
    const JSValue scalar(30);
    EXPECT_FALSE(layer.setProperty("test-constrained-array", conversion::Convertible(&scalar)));

    JSDocument tooLong;
    tooLong.Parse("[0, 1, 2, 3, 4]");
    const JSValue* tooLongValue = &tooLong;
    EXPECT_TRUE(layer.setProperty("test-constrained-array", conversion::Convertible(tooLongValue)));

    JSDocument outOfRange;
    outOfRange.Parse("[91]");
    const JSValue* outOfRangeValue = &outOfRange;
    EXPECT_TRUE(layer.setProperty("test-constrained-array", conversion::Convertible(outOfRangeValue)));

    Descriptor enumeration("org.maplibre.test.constrained-enum", "test-constrained-enum");
    enumeration.defaultValue.type = MLN_PLUGIN_VALUE_STRING;
    enumeration.defaultValue.data.string_value = cString("first");
    enumeration.property.type = MLN_PLUGIN_VALUE_STRING;
    enumeration.property.default_value = enumeration.defaultValue;
    const std::array<mln_plugin_string, 2> allowed{cString("first"), cString("second")};
    enumeration.property.enum_values = allowed.data();
    enumeration.property.enum_value_count = allowed.size();
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&enumeration.descriptor, nullptr, 0));

    const JSValue allowedValue("second");
    EXPECT_FALSE(layer.setProperty("test-constrained-enum", conversion::Convertible(&allowedValue)));
    const JSValue rejectedValue("third");
    EXPECT_TRUE(layer.setProperty("test-constrained-enum", conversion::Convertible(&rejectedValue)));
}

TEST(PluginStyleProperty, ParsesSerializesAndValidatesColorRamps) {
    constexpr char defaultRamp[] =
        R"JSON(["interpolate",["linear"],["heatmap-density"],0,"rgba(0, 0, 255, 0)",1,"red"])JSON";
    Descriptor ramp("org.maplibre.test.color-ramp", "test-color-ramp");
    ramp.defaultValue.type = MLN_PLUGIN_VALUE_COLOR_RAMP;
    ramp.defaultValue.data.color_ramp_json = {defaultRamp, sizeof(defaultRamp) - 1};
    ramp.property.type = MLN_PLUGIN_VALUE_COLOR_RAMP;
    ramp.property.default_value = ramp.defaultValue;
    ramp.property.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&ramp.descriptor, nullptr, 0));

    FillExtrusionLayer layer("color-ramp", "source");
    JSDocument expression;
    expression.Parse(R"JSON(["interpolate",["linear"],["heatmap-density"],0,"rgba(0, 255, 0, 0)",1,"yellow"])JSON");
    const JSValue* expressionValue = &expression;
    EXPECT_FALSE(layer.setProperty("test-color-ramp", conversion::Convertible(expressionValue)));
    const auto serialized = static_cast<const Layer&>(layer).serialize();
    const auto* paint = serialized.getObject()->at("paint").getObject();
    ASSERT_NE(nullptr, paint);
    EXPECT_NE(nullptr, paint->at("test-color-ramp").getArray());

    constexpr char invalidRamp[] = R"JSON(["not-a-color-ramp"])JSON";
    Descriptor invalid("org.maplibre.test.invalid-color-ramp", "test-invalid-color-ramp");
    invalid.defaultValue.type = MLN_PLUGIN_VALUE_COLOR_RAMP;
    invalid.defaultValue.data.color_ramp_json = {invalidRamp, sizeof(invalidRamp) - 1};
    invalid.property.type = MLN_PLUGIN_VALUE_COLOR_RAMP;
    invalid.property.default_value = invalid.defaultValue;
    invalid.property.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&invalid.descriptor, nullptr, 0));
}
