#include <mln/plugin/plugin_registry.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/style/layers/symbol_layer_impl.hpp>
#include <mln/style/layers/symbol_layer_properties.hpp>
#include <mln/style/image_impl.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/layer.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/storage/response.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/text/glyph_manager.hpp>
#include <mln/tile/geojson_tile.hpp>
#include <mln/util/run_loop.hpp>
#include <gtest/gtest.h>

#include <array>
#include <cstdlib>
#include <map>
#include <cstring>
#include <memory>

using namespace mln;

namespace {

mln_plugin_string view(const std::string& value) {
    return {value.data(), value.size()};
}

// Only the size prefix is accessible. ASan catches any payload read before
// rejection, even if that read would not normally dereference a bad pointer.
template <class T>
class ShortStruct {
public:
    const T* get() const { return reinterpret_cast<const T*>(size.get()); }

private:
    std::unique_ptr<uint32_t> size = std::make_unique<uint32_t>(sizeof(uint32_t));
};

struct Descriptor {
    explicit Descriptor(std::string id_)
        : id(std::move(id_)),
          type(id + ".layer") {
        property.struct_size = sizeof(property);
        property.name = view(propertyName);
        property.type = MLN_PLUGIN_VALUE_STRING;
        property.default_value = {sizeof(mln_plugin_value), MLN_PLUGIN_VALUE_STRING, {}};
        property.default_value.data.string_value = view(enumValue);
        enumValues = {view(enumValue)};
        property.enum_values = enumValues.data();
        property.enum_value_count = enumValues.size();
        source = {sizeof(source), MLN_PLUGIN_BACKEND_OPENGL, view(vertex), view(fragment), {}, {}};
        attributes = {{sizeof(mln_plugin_shader_attribute_v1), 0, 0, view(positionName), MLN_PLUGIN_VERTEX_INT16_X2},
                      {sizeof(mln_plugin_shader_attribute_v1), 1, 1, view(paintName), MLN_PLUGIN_VERTEX_FLOAT_X2}};
        uniform = {
            sizeof(uniform), 0, view(uniformName), 16, MLN_PLUGIN_SHADER_STAGE_VERTEX, MLN_PLUGIN_UNIFORM_DRAWABLE};
        binding = {sizeof(binding), view(propertyName), MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT, 0, 0, 1, 1, 0, 4};
        shader = {sizeof(shader),
                  view(shaderID),
                  &source,
                  1,
                  attributes.data(),
                  attributes.size(),
                  &uniform,
                  1,
                  &binding,
                  1,
                  0,
                  0};
        layer.struct_size = sizeof(layer);
        layer.layer_type = view(type);
        layer.backend_mask = MLN_PLUGIN_BACKEND_OPENGL;
        layer.properties = &property;
        layer.property_count = 1;
        layer.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POINT;
        layer.shaders = &shader;
        layer.shader_count = 1;
        layer.create_layout = [](const mln_plugin_layout_context_v1*, void**) {
            return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
        };
        layer.layout_feature = [](void*, const mln_plugin_feature_v1*) {
            return MLN_PLUGIN_STATUS_OK;
        };
        layer.finish_layout = [](void*, mln_plugin_bucket_v1*) {
            return MLN_PLUGIN_STATUS_OK;
        };
        layer.destroy_layout = [](void*) {
        };
        layer.update_uniform_block = [](const mln_plugin_uniform_context_v1*, uint32_t, uint8_t*, size_t) {
            return MLN_PLUGIN_STATUS_OK;
        };
        descriptor = {sizeof(descriptor), MLN_PLUGIN_ABI_VERSION_1, view(id), view(version), 1, 1, &layer, 1};
    }

    void expectRejected(mln_plugin_status expected = MLN_PLUGIN_STATUS_INVALID_ARGUMENT) const {
        char error[256]{};
        EXPECT_EQ(expected, mln_plugin_register_v1(&descriptor, error, sizeof(error))) << error;
        EXPECT_NE('\0', error[0]);
        EXPECT_FALSE(plugin::PluginRegistry::get().findLayerType(type));
        EXPECT_FALSE(LayerManager::get()->hasLayerType(type));
    }

    std::string id, type;
    std::string version = "test-version";
    std::string propertyName = "test-anchor", enumValue = "viewport";
    std::string shaderID = "test-shader", positionName = "a_pos", paintName = "a_anchor", uniformName = "TestUBO";
    std::string vertex = "vertex source", fragment = "fragment source";
    std::vector<mln_plugin_string> enumValues;
    std::vector<mln_plugin_shader_attribute_v1> attributes;
    mln_plugin_property_descriptor_v1 property{};
    mln_plugin_shader_source_v1 source{};
    mln_plugin_uniform_block_descriptor_v1 uniform{};
    mln_plugin_shader_property_binding_v1 binding{};
    mln_plugin_shader_descriptor_v1 shader{};
    mln_plugin_layer_type_v1 layer{};
    mln_plugin_descriptor_v1 descriptor{};
};

TEST(PluginApi, RejectsTruncatedDescriptorsBeforeReadingPayload) {
    ShortStruct<mln_plugin_descriptor_v1> root;
    EXPECT_EQ(MLN_PLUGIN_STATUS_UNSUPPORTED_ABI, mln_plugin_register_v1(root.get(), nullptr, 0));

    Descriptor input("test.truncated");
    ShortStruct<mln_plugin_layer_type_v1> layer;
    input.descriptor.layer_types = layer.get();
    input.expectRejected();
    input.descriptor.layer_types = &input.layer;
    ShortStruct<mln_plugin_shader_descriptor_v1> shader;
    input.layer.shaders = shader.get();
    input.expectRejected();
    input.layer.shaders = &input.shader;
    ShortStruct<mln_plugin_shader_source_v1> source;
    input.shader.sources = source.get();
    input.expectRejected();
    input.shader.sources = &input.source;
    ShortStruct<mln_plugin_shader_attribute_v1> attribute;
    input.shader.attributes = attribute.get();
    input.expectRejected();
    input.shader.attributes = input.attributes.data();
    ShortStruct<mln_plugin_uniform_block_descriptor_v1> uniform;
    input.shader.uniform_blocks = uniform.get();
    input.expectRejected();
    input.shader.uniform_blocks = &input.uniform;
    ShortStruct<mln_plugin_property_descriptor_v1> property;
    input.layer.properties = property.get();
    input.expectRejected();
    input.layer.properties = &input.property;
    input.property.default_value.struct_size = sizeof(uint32_t);
    input.expectRejected();
    input.property.default_value.struct_size = sizeof(mln_plugin_value);
    ShortStruct<mln_plugin_shader_property_binding_v1> binding;
    input.shader.property_bindings = binding.get();
    input.expectRejected();
}

TEST(PluginApi, RejectsMissingArraysAndStringData) {
    Descriptor input("test.missing-arrays");
    input.descriptor.layer_types = nullptr;
    input.expectRejected();
    input.descriptor.layer_types = &input.layer;
    input.layer.shaders = nullptr;
    input.expectRejected();
    input.layer.shaders = &input.shader;
    input.shader.sources = nullptr;
    input.expectRejected();
    input.shader.sources = &input.source;
    input.shader.attributes = nullptr;
    input.expectRejected();
    input.shader.attributes = input.attributes.data();
    input.shader.uniform_blocks = nullptr;
    input.expectRejected();
    input.shader.uniform_blocks = &input.uniform;
    input.shader.property_bindings = nullptr;
    input.expectRejected();
    input.shader.property_bindings = &input.binding;
    input.layer.properties = nullptr;
    input.expectRejected();
    input.layer.properties = &input.property;
    input.property.enum_values = nullptr;
    input.expectRejected();
    input.property.enum_values = input.enumValues.data();
    // Optional shader strings must also have data when their size is nonzero.
    input.source.vertex_entry_point = {nullptr, 1};
    input.expectRejected();
    input.source.vertex_entry_point = {};
    input.source.fragment_entry_point = {nullptr, 1};
    input.expectRejected();
    input.source.fragment_entry_point = {};
    input.source.backend = MLN_PLUGIN_BACKEND_METAL;
    input.layer.backend_mask = MLN_PLUGIN_BACKEND_METAL;
    input.source.vertex_entry_point = view(input.vertex);
    input.source.fragment_entry_point = view(input.fragment);
    input.source.fragment_source = {nullptr, 1};
    input.expectRejected();
    input.source.fragment_source = {};
    input.property.enum_value_count = 0;
    input.shader.property_binding_count = 0;
    input.property.default_value.data.string_value = {nullptr, 1};
    input.expectRejected();
}

TEST(PluginApi, ValidatesVertexAttributeLimits) {
    Descriptor input("test.attribute-limits");
    auto& position = input.attributes.front();
    position.attribute_id = MLN_PLUGIN_MAX_VERTEX_ATTRIBUTES;
    input.expectRejected();
    position.attribute_id = MLN_PLUGIN_MAX_VERTEX_ATTRIBUTES - 1;
    position.location = MLN_PLUGIN_MAX_VERTEX_ATTRIBUTES;
    input.expectRejected();
    position.location = 0;
    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
}

TEST(PluginApi, RejectsShortPropertyBindingWithoutDereferencingItsName) {
    Descriptor input("test.short-binding");
    input.binding.struct_size = sizeof(uint32_t);
    input.binding.property_name = {reinterpret_cast<const char*>(uintptr_t{1}), 1};
    // Isolate an accidental bad read so it becomes a test failure, not a crash
    // that prevents the other lifecycle tests from running.
    ASSERT_EXIT(
        {
            const auto status = mln_plugin_register_v1(&input.descriptor, nullptr, 0);
            std::_Exit(status == MLN_PLUGIN_STATUS_INVALID_ARGUMENT ? 0 : 1);
        },
        ::testing::ExitedWithCode(0),
        "");
}

TEST(PluginApi, RegistrationIsAtomicAndCopiesAllMetadata) {
    {
        Descriptor input("test.copied-metadata");
        auto secondLayer = input.layer;
        std::string secondType = "test.copied-metadata.second";
        secondLayer.layer_type = view(secondType);
        secondLayer.create_layout = nullptr;
        mln_plugin_layer_type_v1 layers[] = {input.layer, secondLayer};
        input.descriptor.layer_types = layers;
        input.descriptor.layer_type_count = 2;
        input.expectRejected();
        EXPECT_FALSE(LayerManager::get()->hasLayerType(secondType));
        // A later type that conflicts with a type of another plugin rejects the whole descriptor.
        Descriptor other("test.copied-metadata.other");
        ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&other.descriptor, nullptr, 0));
        layers[1] = input.layer;
        layers[1].layer_type = view(other.type);
        input.expectRejected(MLN_PLUGIN_STATUS_CONFLICT);
        // So does a later type with the name of a built-in layer, which a plugin cannot replace.
        layers[1].layer_type = {"circle", 6};
        input.expectRejected(MLN_PLUGIN_STATUS_CONFLICT);
        input.descriptor.layer_types = &input.layer;
        input.descriptor.layer_type_count = 1;
        ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
        // Corrupt every string backing the descriptor before freeing its arrays.
        for (auto* value : {&input.id,
                            &input.type,
                            &input.version,
                            &input.propertyName,
                            &input.enumValue,
                            &input.shaderID,
                            &input.positionName,
                            &input.paintName,
                            &input.uniformName,
                            &input.vertex,
                            &input.fragment}) {
            value->assign(value->size(), '!');
        }
    }
    const auto layer = plugin::PluginRegistry::get().findLayerType("test.copied-metadata.layer");
    ASSERT_TRUE(layer);
    EXPECT_EQ("test.copied-metadata", layer->pluginID);
    EXPECT_STREQ("test.copied-metadata.layer", layer->info.type);
    ASSERT_EQ(1u, layer->shaders.size());
    const auto& shader = layer->shaders.front();
    EXPECT_EQ("test-shader", shader.id);
    EXPECT_EQ("vertex source", shader.sources.front().vertex);
    EXPECT_EQ("fragment source", shader.sources.front().fragment);
    EXPECT_EQ("a_pos", shader.attributes.front().name);
    EXPECT_EQ("a_anchor", shader.attributes.back().name);
    EXPECT_EQ("TestUBO", shader.uniformBlocks.front().name);
    EXPECT_EQ("test-anchor", shader.propertyBindings.front().propertyName);
    const auto property = layer->findProperty("test-anchor");
    ASSERT_TRUE(property);
    ASSERT_NE(nullptr, property->defaultValue.getString());
    EXPECT_EQ("viewport", *property->defaultValue.getString());
    EXPECT_EQ(std::vector<std::string>{"viewport"}, property->enumValues);
    Descriptor repeated("test.copied-metadata");
    EXPECT_EQ(MLN_PLUGIN_STATUS_ALREADY_REGISTERED, mln_plugin_register_v1(&repeated.descriptor, nullptr, 0));
    EXPECT_EQ(layer, plugin::PluginRegistry::get().findLayerType(layer->type));
    style::PluginStyleLayer original("original", "points", layer);
    auto clone = static_cast<const style::Layer&>(original).cloneRef("clone");
    EXPECT_EQ(original.getTypeInfo(), clone->getTypeInfo());
    EXPECT_EQ(layer, static_cast<style::PluginStyleLayer&>(*clone).impl().registration);
}

TEST(PluginApi, RepeatedRegistrationComparesNestedDefinitions) {
    Descriptor original("test.structural-equality");
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&original.descriptor, nullptr, 0));
    const auto retained = plugin::PluginRegistry::get().findLayerType(original.type);
    const auto check = [&](auto mutate) {
        Descriptor changed(original.id);
        mutate(changed);
        char error[256]{};
        EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&changed.descriptor, error, sizeof(error)))
            << error;
        EXPECT_EQ(retained, plugin::PluginRegistry::get().findLayerType(original.type));
    };
    check([](auto& d) { d.descriptor.plugin_version = {"other", 5}; });
    check([](auto& d) { d.source.fragment_source = {"different shader", 16}; });
    check([](auto& d) { d.attributes.front().name = {"different", 9}; });
    check([](auto& d) { d.uniform.stage_mask |= MLN_PLUGIN_SHADER_STAGE_FRAGMENT; });
    check([](auto& d) { d.uniform.scope = MLN_PLUGIN_UNIFORM_DRAWABLE_ARRAY; });
    check([](auto& d) { d.binding.uniform_byte_offset = 8; });
    check([](auto& d) { d.property.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA; });
    check([](auto& d) { d.layer.geometry_type_mask |= MLN_PLUGIN_GEOMETRY_POLYGON; });
    check([](auto& d) { d.layer.is_3d = 1; });
    check([](auto& d) { d.layer.enable_near_clipped_matrix = 1; });
    check([](auto& d) {
        d.layer.query_feature = [](const mln_plugin_feature_v1*,
                                   const mln_plugin_tile_point_v1*,
                                   size_t,
                                   const mln_plugin_query_context_v1*,
                                   const mln_plugin_property_value_v1*,
                                   size_t) -> uint8_t {
            return 0;
        };
    });
}

TEST(PluginApi, CopiesRenderingModeFlags) {
    Descriptor withDefaults("test.render-flags.default");
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&withDefaults.descriptor, nullptr, 0));
    const auto defaultLayer = plugin::PluginRegistry::get().findLayerType(withDefaults.type);
    ASSERT_TRUE(defaultLayer);
    EXPECT_FALSE(defaultLayer->is3D);
    EXPECT_FALSE(defaultLayer->enableNearClippedMatrix);

    Descriptor withFlagsEnabled("test.render-flags.enabled");
    withFlagsEnabled.layer.is_3d = 1;
    withFlagsEnabled.layer.enable_near_clipped_matrix = 1;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&withFlagsEnabled.descriptor, nullptr, 0));
    const auto enabledLayer = plugin::PluginRegistry::get().findLayerType(withFlagsEnabled.type);
    ASSERT_TRUE(enabledLayer);
    EXPECT_TRUE(enabledLayer->is3D);
    EXPECT_TRUE(enabledLayer->enableNearClippedMatrix);
}

TEST(PluginApi, ValidatesAndCopiesOrderedRenderingDescriptors) {
    Descriptor input("test.ordered-passes");
    mln_plugin_draw_pass_v1 passes[] = {{sizeof(mln_plugin_draw_pass_v1), 1, 1, 0, 0, 0, MLN_PLUGIN_CULL_BACK_CCW},
                                        {sizeof(mln_plugin_draw_pass_v1), 1, 0, 1, 1, 1, MLN_PLUGIN_CULL_BACK_CCW}};
    input.layer.draw_passes = passes;
    input.layer.draw_pass_count = 2;
    input.expectRejected(); // Layer stencil requires an explicit 3D layer.
    input.layer.is_3d = 1;
    passes[0].struct_size = 4;
    input.expectRejected();
    passes[0].struct_size = sizeof(passes[0]);
    passes[0].depth_test = 0;
    input.expectRejected(); // Writing depth without testing is invalid.
    passes[0].depth_test = 1;
    passes[0].color_write = 2;
    input.expectRejected();
    passes[0].color_write = 0;
    input.layer.draw_pass_count = 33;
    input.expectRejected();
    input.layer.draw_pass_count = 2;
    input.layer.draw_passes = nullptr;
    input.expectRejected();
    input.layer.draw_passes = passes;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
    const auto registered = plugin::PluginRegistry::get().findLayerType(input.type);
    ASSERT_EQ(2u, registered->drawPasses.size());
    EXPECT_FALSE(registered->drawPasses[0].colorWrite);
    EXPECT_TRUE(registered->drawPasses[1].stencilDedup);
    EXPECT_FALSE(registered->drawPasses[1].depthWrite);
    EXPECT_EQ(style::LayerTypeInfo::Pass3D::Required, registered->info.pass3d);
    passes[1].depth_write = 1;
    EXPECT_FALSE(registered->drawPasses[1].depthWrite); // Borrowed array was copied.
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
}

TEST(PluginApi, BooleanDefaultsAndShaderEncodingAreValidated) {
    Descriptor input("test.boolean-property");
    input.property.type = MLN_PLUGIN_VALUE_BOOLEAN;
    input.property.default_value.type = MLN_PLUGIN_VALUE_BOOLEAN;
    input.property.default_value.data.boolean_value = 1;
    input.property.enum_value_count = 0;
    input.binding.encoding = MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT;
    input.property.default_value.data.boolean_value = 2;
    input.expectRejected();
    input.property.default_value.data.boolean_value = 1;
    input.binding.encoding = MLN_PLUGIN_PROPERTY_ENCODING_FLOAT;
    input.expectRejected();
    input.binding.encoding = MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
}

TEST(PluginApi, ImagePropertiesCannotDependOnFeatureState) {
    Descriptor input("test.image-feature-state");
    input.property.type = MLN_PLUGIN_VALUE_IMAGE;
    input.property.default_value.type = MLN_PLUGIN_VALUE_IMAGE;
    input.property.enum_value_count = 0;
    input.property.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
    input.shader.property_binding_count = 0;
    char error[256]{};
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&input.descriptor, error, sizeof(error)));
    EXPECT_NE(nullptr, std::strstr(error, "feature state")) << error;
    input.property.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, error, sizeof(error))) << error;
}

TEST(PluginApi, LayoutPropertiesValidateScopeSerializeAndInvalidateGeometry) {
    Descriptor input("test.layout-property");
    input.property.is_layout = 1;
    input.property.type = MLN_PLUGIN_VALUE_FLOAT;
    input.property.default_value.type = MLN_PLUGIN_VALUE_FLOAT;
    input.property.default_value.data.float_value = 0;
    input.property.enum_value_count = 0;
    input.property.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    input.shader.property_binding_count = 0;
    input.property.supports_transitions = 1;
    input.expectRejected();
    input.property.supports_transitions = 0;
    input.property.expression_capabilities |= MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
    input.expectRejected();
    // Feature-dependent layout values are evaluated per feature and need no shader binding.
    input.property.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE |
                                             MLN_PLUGIN_EXPRESSION_COMPOSITE;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
    style::conversion::Error error;
    auto parse = [&](const std::string& section, int value) {
        return style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
            "{\"id\":\"test\",\"source\":\"s\",\"type\":\"" + input.type + "\",\"" + section +
                "\":{\"test-anchor\":" + std::to_string(value) + "}}",
            error);
    };
    EXPECT_FALSE(parse("paint", 2));
    auto a = parse("layout", 2), b = parse("layout", 4);
    ASSERT_TRUE(a) << error.message;
    ASSERT_TRUE(b) << error.message;
    const auto& ai = static_cast<style::PluginStyleLayer&>(**a).impl();
    const auto& bi = static_cast<style::PluginStyleLayer&>(**b).impl();
    EXPECT_TRUE(ai.hasLayoutDifference(bi));
    EXPECT_FALSE(ai.hasLayoutDifference(ai));
    const auto serialized = (*a)->serialize();
    ASSERT_TRUE(serialized.getObject()->at("layout").getObject());
    EXPECT_EQ(2.0, *serialized.getObject()->at("layout").getObject()->at("test-anchor").getDouble());
}

} // namespace

TEST(PluginApi, NormalizedColorEncodingValidatesAttributesAndUniformFootprint) {
    Descriptor input("test.rgba8-color");
    input.property.type = MLN_PLUGIN_VALUE_COLOR;
    input.property.default_value.type = MLN_PLUGIN_VALUE_COLOR;
    input.property.default_value.data.color_value = {1, 0.5f, 0, 1};
    input.property.enum_value_count = 0;
    input.binding.encoding = MLN_PLUGIN_PROPERTY_ENCODING_COLOR_RGBA8;
    input.binding.maximum_attribute_id = 2;
    input.binding.interpolation_uniform_byte_offset = 16;
    input.attributes[1].type = MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED;
    auto maximum = input.attributes[1];
    maximum.attribute_id = maximum.location = 2;
    maximum.name = {"a_max", 5};
    input.attributes.push_back(maximum);
    input.shader.attributes = input.attributes.data();
    input.shader.attribute_count = input.attributes.size();
    input.expectRejected(); // Float4 plus interpolation does not fit in 16 bytes.
    input.uniform.byte_size = 32;
    input.attributes[1].type = MLN_PLUGIN_VERTEX_FLOAT_X4;
    input.expectRejected();
    input.attributes[1].type = MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED;
    input.binding.maximum_attribute_id = 1;
    input.expectRejected(); // Two endpoints cannot share one attribute ID.
    input.binding.maximum_attribute_id = 2;
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
}

namespace {

struct ResourceDescriptor : Descriptor {
    explicit ResourceDescriptor(std::string id_)
        : Descriptor(std::move(id_)) {
        for (auto& resourceProperty : properties) {
            resourceProperty.struct_size = sizeof(resourceProperty);
            resourceProperty.is_layout = 1;
        }
        properties[0].name = view(modelsName);
        properties[0].type = MLN_PLUGIN_VALUE_RESOURCES;
        properties[0].default_value = {sizeof(mln_plugin_value), MLN_PLUGIN_VALUE_RESOURCES, {}};
        properties[1].name = view(modelName);
        properties[1].type = MLN_PLUGIN_VALUE_RESOURCE;
        properties[1].default_value = {sizeof(mln_plugin_value), MLN_PLUGIN_VALUE_RESOURCE, {}};
        properties[1].expression_capabilities = MLN_PLUGIN_EXPRESSION_FEATURE;
        properties[1].resources = view(modelsName);
        layer.properties = properties.data();
        layer.property_count = properties.size();
        shader.property_binding_count = 0;
        layer.create_resource = [](mln_plugin_string, const uint8_t*, size_t, void**) {
            return MLN_PLUGIN_STATUS_OK;
        };
        layer.destroy_resource = [](void*) {
        };
    }

    std::string modelsName = "test-models", modelName = "test-model", otherName = "test-other";
    std::array<mln_plugin_property_descriptor_v1, 2> properties{};
};

} // namespace

TEST(PluginApi, ResourcePropertiesValidateTheirDescriptorAndCallbacks) {
    ResourceDescriptor input("test.resources");
    input.layer.create_resource = nullptr;
    input.expectRejected(); // A RESOURCE property needs both callbacks.
    input.layer.create_resource = ResourceDescriptor("test.resources.callbacks").layer.create_resource;

    input.properties[1].resources = {};
    input.expectRejected(); // A RESOURCE property names its RESOURCES property.
    input.properties[1].resources = view(input.otherName);
    input.expectRejected(); // ... which must exist.
    input.properties[1].resources = view(input.modelName);
    input.expectRejected(); // ... and be a RESOURCES property.
    input.properties[1].resources = view(input.modelsName);

    input.properties[0].resources = view(input.modelsName);
    input.expectRejected(); // Only a RESOURCE property names one.
    input.properties[0].resources = {};

    input.properties[0].expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA;
    input.expectRejected(); // RESOURCES takes no expressions.
    input.properties[0].expression_capabilities = MLN_PLUGIN_EXPRESSION_NONE;

    input.properties[1].is_layout = 0;
    input.expectRejected(); // Resource properties are layout properties.
    input.properties[1].is_layout = 1;

    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
    const auto registration = plugin::PluginRegistry::get().findLayerType(input.type);
    ASSERT_TRUE(registration);
    EXPECT_EQ("test-models", registration->findProperty("test-model")->resources);
    EXPECT_TRUE(registration->createResource);
    EXPECT_TRUE(registration->destroyResource);

    // Resource callbacks without a RESOURCE property are rejected too.
    Descriptor plain("test.resources.unused");
    plain.layer.create_resource = input.layer.create_resource;
    plain.layer.destroy_resource = input.layer.destroy_resource;
    plain.expectRejected();
}

TEST(PluginApi, ResourcesPropertyParsesSerializesAndInvalidatesLayout) {
    ResourceDescriptor input("test.resources-style");
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));
    style::conversion::Error error;
    auto parse = [&](const std::string& layout) {
        return style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
            R"({"id":"test","source":"s","type":")" + input.type + R"(","layout":)" + layout + "}", error);
    };
    EXPECT_FALSE(parse(R"({"test-models":"asset://car.glb"})"));
    EXPECT_FALSE(parse(R"({"test-models":{"car":""}})"));
    EXPECT_FALSE(parse(R"({"test-models":{"car":"asset://car.glb"},"test-model":["feature-state","kind"]})"));

    auto a = parse(R"({"test-models":{"car":"asset://car.glb"},"test-model":["get","kind"]})");
    auto b = parse(R"({"test-models":{"car":"asset://truck.glb"},"test-model":["get","kind"]})");
    ASSERT_TRUE(a) << error.message;
    ASSERT_TRUE(b) << error.message;
    const auto& ai = static_cast<style::PluginStyleLayer&>(**a).impl();
    const auto& bi = static_cast<style::PluginStyleLayer&>(**b).impl();
    EXPECT_EQ((std::map<std::string, std::string>{{"car", "asset://car.glb"}}), ai.pluginResources.at("test-models"));
    EXPECT_TRUE(ai.hasLayoutDifference(bi));
    EXPECT_FALSE(ai.hasLayoutDifference(ai));

    const auto property = (*a)->getProperty("test-models");
    EXPECT_EQ(style::StyleProperty::Kind::Constant, property.getKind());
    ASSERT_TRUE(property.getValue().getObject());
    EXPECT_EQ("asset://car.glb", *property.getValue().getObject()->at("car").getString());

    const auto serialized = (*a)->serialize();
    const auto& layout = *serialized.getObject()->at("layout").getObject();
    EXPECT_EQ("asset://car.glb", *layout.at("test-models").getObject()->at("car").getString());
    EXPECT_TRUE(layout.contains("test-model"));
}

namespace {

class PointData final : public style::GeoJSONData {
public:
    explicit PointData(mapbox::feature::feature_collection<int16_t> features_)
        : features(std::move(features_)) {}
    void getTile(const CanonicalTileID&, const std::function<void(TileFeatures)>& fn, bool) override { fn(features); }
    Features getChildren(std::uint32_t) override { return {}; }
    Features getLeaves(std::uint32_t, std::uint32_t, std::uint32_t) override { return {}; }
    std::uint8_t getClusterExpansionZoom(std::uint32_t) override { return 0; }

private:
    TileFeatures features;
};

struct TileResourceRecord {
    std::vector<std::string> created;
    unsigned destroyed = 0;
    std::vector<void*> featureHandles;
} tileResources;

} // namespace

TEST(PluginApi, LaysATileOutOnceTheResourcesItsFeaturesNameHaveLoaded) {
    tileResources = {};
    ResourceDescriptor input("test.resources-tile");
    input.layer.create_layout = [](const mln_plugin_layout_context_v1*, void** instance) {
        static int layout = 0;
        *instance = &layout;
        return MLN_PLUGIN_STATUS_OK;
    };
    input.layer.layout_feature = [](void*, const mln_plugin_feature_v1* feature) {
        EXPECT_EQ(1u, feature->layout_property_count);
        if (feature->layout_property_count == 1) {
            tileResources.featureHandles.push_back(feature->layout_properties[0].value.data.resource_value.handle);
        }
        return MLN_PLUGIN_STATUS_OK;
    };
    input.layer.finish_layout = [](void*, mln_plugin_bucket_v1* output) {
        *output = {};
        output->struct_size = sizeof(*output);
        return MLN_PLUGIN_STATUS_OK;
    };
    input.layer.create_resource = [](mln_plugin_string, const uint8_t* data, size_t size, void** handle) {
        tileResources.created.emplace_back(reinterpret_cast<const char*>(data), size);
        *handle = new std::string(tileResources.created.back());
        return MLN_PLUGIN_STATUS_OK;
    };
    input.layer.destroy_resource = [](void* handle) {
        ++tileResources.destroyed;
        delete static_cast<std::string*>(handle);
    };
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));

    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(StubFileSource::ResponseType::Manual);
    std::vector<std::string> requested;
    fileSource->response = [&](const Resource& resource) -> std::optional<Response> {
        requested.push_back(resource.url);
        Response response;
        response.data = std::make_shared<std::string>("glb of " + resource.url);
        return response;
    };
    TransformState transformState;
    util::SimpleIdentity uniqueID;
    TileParameters parameters{.pixelRatio = 1.0,
                              .debugOptions = MapDebugOptions(),
                              .transformState = transformState,
                              .fileSource = fileSource,
                              .mode = MapMode::Continuous,
                              .annotationManager = {},
                              .imageManager = ImageManager::create(),
                              .glyphManager = std::make_shared<GlyphManager>(),
                              .prefetchZoomDelta = 0,
                              .threadPool = {Scheduler::GetBackground(), uniqueID},
                              .isUpdateSynchronous = true};

    style::conversion::Error error;
    const auto layer = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"models","source":"points","type":")" + input.type +
            R"(","layout":{"test-models":{"car":"asset://car.glb","truck":"asset://truck.glb"},)"
            R"("test-model":["get","kind"]}})",
        error);
    ASSERT_TRUE(layer) << error.message;
    const std::vector<Immutable<style::LayerProperties>> layers{makeMutable<style::PluginStyleLayerProperties>(
        staticImmutableCast<style::PluginStyleLayer::Impl>((*layer)->baseImpl))};

    mapbox::feature::feature_collection<int16_t> features;
    mapbox::feature::feature<int16_t> car{mapbox::geometry::point<int16_t>(100, 100)};
    car.properties = {{"kind", std::string("car")}};
    features.push_back(std::move(car));
    {
        GeoJSONTile tile(OverscaledTileID(0, 0, 0), "points", parameters, std::make_shared<PointData>(features));
        tile.setLayers(layers, nullptr);
        // The tile waits for the model its feature names; the layer's other model is not loaded.
        EXPECT_FALSE(tile.isComplete());
        EXPECT_TRUE(tileResources.featureHandles.empty());

        fileSource->respondToAll();
        EXPECT_EQ((std::vector<std::string>{"asset://car.glb"}), requested);
        EXPECT_TRUE(tile.isComplete());
        EXPECT_EQ((std::vector<std::string>{"glb of asset://car.glb"}), tileResources.created);
        ASSERT_EQ(1u, tileResources.featureHandles.size());
        ASSERT_NE(nullptr, tileResources.featureHandles[0]);
        EXPECT_EQ("glb of asset://car.glb", *static_cast<std::string*>(tileResources.featureHandles[0]));
        EXPECT_EQ(0u, tileResources.destroyed);
    }
    // No tile uses the model any more.
    EXPECT_EQ(1u, tileResources.destroyed);
}

// parse() requests the images before the resources. An image that is already loaded is answered while parse() still
// runs, and that answer must not lay the tile out before the resources are requested and have arrived.
TEST(PluginApi, WaitsForResourcesWhenAnImageIsAnsweredDuringParse) {
    tileResources = {};
    ResourceDescriptor input("test.resources-image");
    input.layer.create_layout = [](const mln_plugin_layout_context_v1*, void** instance) {
        static int layout = 0;
        *instance = &layout;
        return MLN_PLUGIN_STATUS_OK;
    };
    input.layer.layout_feature = [](void*, const mln_plugin_feature_v1* feature) {
        if (feature->layout_property_count == 1) {
            tileResources.featureHandles.push_back(feature->layout_properties[0].value.data.resource_value.handle);
        }
        return MLN_PLUGIN_STATUS_OK;
    };
    input.layer.finish_layout = [](void*, mln_plugin_bucket_v1* output) {
        *output = {};
        output->struct_size = sizeof(*output);
        return MLN_PLUGIN_STATUS_OK;
    };
    input.layer.create_resource = [](mln_plugin_string, const uint8_t* data, size_t size, void** handle) {
        tileResources.created.emplace_back(reinterpret_cast<const char*>(data), size);
        *handle = new std::string(tileResources.created.back());
        return MLN_PLUGIN_STATUS_OK;
    };
    input.layer.destroy_resource = [](void* handle) {
        ++tileResources.destroyed;
        delete static_cast<std::string*>(handle);
    };
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&input.descriptor, nullptr, 0));

    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(StubFileSource::ResponseType::Manual);
    fileSource->response = [&](const Resource& resource) -> std::optional<Response> {
        Response response;
        response.data = std::make_shared<std::string>("glb of " + resource.url);
        return response;
    };
    // The icon is already loaded, so the image manager answers as soon as the tile asks.
    auto imageManager = ImageManager::create();
    imageManager->addImage(makeMutable<style::Image::Impl>("pin", PremultipliedImage({4, 4}), 1.0f));
    imageManager->setLoaded(true);
    TransformState transformState;
    util::SimpleIdentity uniqueID;
    TileParameters parameters{.pixelRatio = 1.0,
                              .debugOptions = MapDebugOptions(),
                              .transformState = transformState,
                              .fileSource = fileSource,
                              .mode = MapMode::Continuous,
                              .annotationManager = {},
                              .imageManager = imageManager,
                              .glyphManager = std::make_shared<GlyphManager>(),
                              .prefetchZoomDelta = 0,
                              .threadPool = {Scheduler::GetBackground(), uniqueID},
                              .isUpdateSynchronous = true};

    style::conversion::Error error;
    const auto models = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"models","source":"points","type":")" + input.type +
            R"(","layout":{"test-models":{"car":"asset://car.glb"},"test-model":["get","kind"]}})",
        error);
    ASSERT_TRUE(models) << error.message;
    const auto pins = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"pins","source":"points","type":"symbol","layout":{"icon-image":"pin"}})", error);
    ASSERT_TRUE(pins) << error.message;
    const std::vector<Immutable<style::LayerProperties>> layers{
        makeMutable<style::PluginStyleLayerProperties>(
            staticImmutableCast<style::PluginStyleLayer::Impl>((*models)->baseImpl)),
        makeMutable<style::SymbolLayerProperties>(staticImmutableCast<style::SymbolLayer::Impl>((*pins)->baseImpl))};

    mapbox::feature::feature_collection<int16_t> features;
    mapbox::feature::feature<int16_t> car{mapbox::geometry::point<int16_t>(100, 100)};
    car.properties = {{"kind", std::string("car")}};
    features.push_back(std::move(car));
    {
        GeoJSONTile tile(OverscaledTileID(0, 0, 0), "points", parameters, std::make_shared<PointData>(features));
        tile.setLayers(layers, nullptr);
        // The image answered during parse(), but the model has not arrived, so nothing is laid out yet.
        EXPECT_FALSE(tile.isComplete());
        EXPECT_TRUE(tileResources.featureHandles.empty());

        fileSource->respondToAll();
        EXPECT_TRUE(tile.isComplete());
        ASSERT_EQ(1u, tileResources.featureHandles.size());
        ASSERT_NE(nullptr, tileResources.featureHandles[0]);
        EXPECT_EQ("glb of asset://car.glb", *static_cast<std::string*>(tileResources.featureHandles[0]));
    }
    EXPECT_EQ(1u, tileResources.destroyed);
}
