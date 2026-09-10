#include <mln/plugin/plugin_api.h>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/layer.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <gtest/gtest.h>

using namespace mln;

namespace {
const mln_plugin_descriptor_v1* testDescriptor() {
    // Registration/style tests need metadata, not the example plugin or working shaders.
    static const mln_plugin_property_descriptor_v1 properties[] = {
        [] {
            mln_plugin_property_descriptor_v1 property{};
            property.struct_size = sizeof(property);
            property.name = {"test-radius", 11};
            property.type = MLN_PLUGIN_VALUE_FLOAT;
            property.default_value = {sizeof(mln_plugin_value), MLN_PLUGIN_VALUE_FLOAT, {.float_value = 5}};
            property.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE |
                                               MLN_PLUGIN_EXPRESSION_COMPOSITE;
            return property;
        }(),
        [] {
            mln_plugin_property_descriptor_v1 property{};
            property.struct_size = sizeof(property);
            property.name = {"test-opacity", 12};
            property.type = MLN_PLUGIN_VALUE_FLOAT;
            property.default_value = {sizeof(mln_plugin_value), MLN_PLUGIN_VALUE_FLOAT, {.float_value = 1}};
            return property;
        }()};
    static const mln_plugin_shader_source_v1 source = {
        sizeof(source), MLN_PLUGIN_BACKEND_OPENGL, {"vertex", 6}, {"fragment", 8}, {}, {}};
    static const mln_plugin_shader_attribute_v1 attributes[] = {
        {sizeof(mln_plugin_shader_attribute_v1), 0, 0, {"a_pos", 5}, MLN_PLUGIN_VERTEX_INT16_X2},
        {sizeof(mln_plugin_shader_attribute_v1), 1, 1, {"a_radius", 8}, MLN_PLUGIN_VERTEX_FLOAT_X2}};
    static const mln_plugin_uniform_block_descriptor_v1 uniform = {
        sizeof(uniform), 0, {"TestUBO", 7}, 16, MLN_PLUGIN_SHADER_STAGE_VERTEX};
    static const mln_plugin_shader_property_binding_v1 binding = {
        sizeof(binding), {"test-radius", 11}, MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 1, 1, 0, 4};
    static const mln_plugin_shader_descriptor_v1 shader = {
        sizeof(shader), {"test", 4}, &source, 1, attributes, 2, &uniform, 1, &binding, 1};
    static const mln_plugin_layer_type_v1 layer = [] {
        mln_plugin_layer_type_v1 value{};
        value.struct_size = sizeof(value);
        value.layer_type = {"test-marker", 11};
        value.backend_mask = MLN_PLUGIN_BACKEND_OPENGL;
        value.properties = properties;
        value.property_count = 2;
        value.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POINT;
        value.shaders = &shader;
        value.shader_count = 1;
        value.create_layout = [](const mln_plugin_layout_context_v1*, void**) {
            return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
        };
        value.layout_feature = [](void*, const mln_plugin_feature_v1*) {
            return MLN_PLUGIN_STATUS_OK;
        };
        value.finish_layout = [](void*, mln_plugin_bucket_v1*) {
            return MLN_PLUGIN_STATUS_OK;
        };
        value.destroy_layout = [](void*) {
        };
        value.update_uniform_block = [](const mln_plugin_uniform_context_v1*, uint32_t, uint8_t*, size_t) {
            return MLN_PLUGIN_STATUS_OK;
        };
        return value;
    }();
    static const mln_plugin_descriptor_v1 descriptor = {
        sizeof(descriptor), MLN_PLUGIN_ABI_VERSION_1, {"test.registry", 13}, {"1", 1}, 1, 1, &layer, 1};
    return &descriptor;
}

TEST(PluginRegistry, ValidatesAndCopiesDescriptors) {
    style::conversion::Error parsingError;
    EXPECT_FALSE(style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"unregistered-plugin","source":"points"})", parsingError));
    const auto* descriptor = testDescriptor();
    ASSERT_NE(nullptr, descriptor);
    char error[256]{};
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(nullptr, error, sizeof(error)));
    EXPECT_NE('\0', error[0]);

    auto input = *descriptor;
    input.abi_version = 2;
    EXPECT_EQ(MLN_PLUGIN_STATUS_UNSUPPORTED_ABI, mln_plugin_register_v1(&input, error, sizeof(error)));
    input = *descriptor;
    input.struct_size = 0;
    EXPECT_EQ(MLN_PLUGIN_STATUS_UNSUPPORTED_ABI, mln_plugin_register_v1(&input, error, sizeof(error)));
    input = *descriptor;
    auto layer = input.layer_types[0];
    input.layer_types = &layer;
    layer.create_layout = nullptr;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&input, error, sizeof(error)));
    layer = descriptor->layer_types[0];
    auto property = layer.properties[0];
    property.default_value.type = MLN_PLUGIN_VALUE_STRING;
    layer.properties = &property;
    layer.property_count = 1;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&input, error, sizeof(error)));

    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(descriptor, error, sizeof(error))) << error;
    EXPECT_EQ(MLN_PLUGIN_STATUS_ALREADY_REGISTERED, mln_plugin_register_v1(descriptor, error, sizeof(error))) << error;
    EXPECT_TRUE(LayerManager::get()->hasLayerType("test-marker"));
    input = *descriptor;
    input.plugin_version = {"different", 9};
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&input, error, sizeof(error)));
    input = *descriptor;
    input.plugin_id = {"another-plugin", 14};
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&input, error, sizeof(error)));
}

TEST(PluginStyle, ParsingDefaultsExpressionsAndClone) {
    char diagnostic[256]{};
    mln_plugin_register_v1(testDescriptor(), diagnostic, sizeof(diagnostic));
    style::conversion::Error error;
    auto parsed = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"points","type":"test-marker","source":"points","paint":{"test-radius":12,"test-opacity":0}})", error);
    ASSERT_TRUE(parsed) << error.message;
    auto& layer = **parsed;
    EXPECT_EQ(12.0, *layer.getProperty("test-radius").getValue().getDouble());
    EXPECT_EQ(0.0, *layer.getProperty("test-opacity").getValue().getDouble());
    auto cloned = layer.cloneRef("clone");
    EXPECT_EQ(layer.getProperty("test-opacity").getValue(), cloned->getProperty("test-opacity").getValue());
    const auto serialized = layer.serialize();
    ASSERT_NE(nullptr, serialized.getObject());
    EXPECT_NE(serialized.getObject()->end(), serialized.getObject()->find("paint"));

    auto wrong = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"test-marker","source":"points","paint":{"test-radius":"wide"}})", error);
    EXPECT_FALSE(wrong);
    wrong = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"circle","source":"points","paint":{"test-radius":12}})", error);
    EXPECT_FALSE(wrong); // Plugins cannot add properties to built-in layers.
    wrong = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"test-marker","source":"points","layout":{"test-radius":12}})", error);
    EXPECT_FALSE(wrong);
    wrong = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(R"({"id":"p","type":"test-marker"})", error);
    EXPECT_FALSE(wrong);
    auto dynamic = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"test-marker","source":"points","paint":{"test-radius":["interpolate",["linear"],["zoom"],10,["get","small"],11,["get","large"]]}})",
        error);
    EXPECT_TRUE(dynamic) << error.message;
    auto filtered = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"points","type":"test-marker","source":"points","filter":["==","n",3]})", error);
    ASSERT_TRUE(filtered) << error.message;
    EXPECT_TRUE(static_cast<const style::PluginStyleLayer&>(layer).impl().hasLayoutDifference(
        static_cast<const style::PluginStyleLayer&>(**filtered).impl()));
}
} // namespace
