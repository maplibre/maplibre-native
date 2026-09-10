#include <mln/plugin/plugin_registry.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <gtest/gtest.h>

#include <cstdlib>
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
        uniform = {sizeof(uniform), 0, view(uniformName), 16, MLN_PLUGIN_SHADER_STAGE_VERTEX};
        binding = {sizeof(binding), view(propertyName), MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT, 0, 0, 1, 1, 0, 4};
        shader = {
            sizeof(shader), view(shaderID), &source, 1, attributes.data(), attributes.size(), &uniform, 1, &binding, 1};
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
        EXPECT_FALSE(plugin::PluginRegistry::get().findProperty(type, propertyName));
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
        // Factory publication must also be atomic when a later type conflicts
        // with a built-in layer rather than failing descriptor validation.
        layers[1] = input.layer;
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
    EXPECT_EQ("test-version", layer->pluginVersion);
    EXPECT_STREQ("test.copied-metadata.layer", layer->identity->info.type);
    ASSERT_EQ(1u, layer->shaders.size());
    const auto& shader = layer->shaders.front();
    EXPECT_EQ("test-shader", shader.id);
    EXPECT_EQ("vertex source", shader.sources.front().vertex);
    EXPECT_EQ("fragment source", shader.sources.front().fragment);
    EXPECT_EQ("a_pos", shader.attributes.front().name);
    EXPECT_EQ("a_anchor", shader.attributes.back().name);
    EXPECT_EQ("TestUBO", shader.uniformBlocks.front().name);
    EXPECT_EQ("test-anchor", shader.propertyBindings.front().propertyName);
    const auto property = plugin::PluginRegistry::get().findProperty(layer->type, "test-anchor");
    ASSERT_TRUE(property);
    ASSERT_NE(nullptr, property->defaultValue.getString());
    EXPECT_EQ("viewport", *property->defaultValue.getString());
    EXPECT_EQ(std::vector<std::string>{"viewport"}, property->enumValues);
    Descriptor repeated("test.copied-metadata");
    EXPECT_EQ(MLN_PLUGIN_STATUS_ALREADY_REGISTERED, mln_plugin_register_v1(&repeated.descriptor, nullptr, 0));
}

} // namespace
