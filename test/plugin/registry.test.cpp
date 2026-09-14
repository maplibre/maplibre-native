#include <mln/plugin/plugin_api.h>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/layer.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/tile/geojson_tile_data.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/circle_layer.hpp>
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
        sizeof(uniform), 0, {"TestUBO", 7}, 16, MLN_PLUGIN_SHADER_STAGE_VERTEX, MLN_PLUGIN_UNIFORM_DRAWABLE};
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

TEST(PluginRegistry, ValidatesUniformScopes) {
    auto descriptor = *testDescriptor();
    descriptor.plugin_id = {"test.uniform-scopes", 19};
    auto layer = descriptor.layer_types[0];
    layer.layer_type = {"test-uniform-scopes", 19};
    descriptor.layer_types = &layer;
    auto shader = layer.shaders[0];
    layer.shaders = &shader;
    auto uniform = shader.uniform_blocks[0];
    shader.uniform_blocks = &uniform;
    char message[256]{};
    uniform.scope = static_cast<mln_plugin_uniform_scope_v1>(99);
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&descriptor, message, sizeof(message)));
    uniform.scope = MLN_PLUGIN_UNIFORM_LAYER;
    // Interpolation factors vary with bucket zoom; they cannot be layer-wide.
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&descriptor, message, sizeof(message)));
    uniform.scope = MLN_PLUGIN_UNIFORM_DRAWABLE_ARRAY;
    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&descriptor, message, sizeof(message))) << message;
    EXPECT_EQ(MLN_PLUGIN_STATUS_ALREADY_REGISTERED, mln_plugin_register_v1(&descriptor, message, sizeof(message)));
    uniform.scope = MLN_PLUGIN_UNIFORM_DRAWABLE;
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&descriptor, message, sizeof(message)));
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

TEST(PluginRegistry, BuiltinOverridePreservesExistingLayerIdentity) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    // Registration is process-wide and permanent; do not replace circle for
    // unrelated core tests in this process.
    ASSERT_EXIT(
        ([] {
            auto parseCircle = [] {
                style::conversion::Error error;
                return style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
                    R"({"id":"circle","type":"circle","source":"points"})", error);
            };
            auto before = parseCircle();
            if (!before) std::exit(1);
            const auto* builtinIdentity = (*before)->getTypeInfo();
            auto descriptor = *testDescriptor();
            auto type = descriptor.layer_types[0];
            type.layer_type = {"circle", 6};
            descriptor.layer_types = &type;
            descriptor.plugin_id = {"test.override", 13};
            char error[256]{};
            if (mln_plugin_register_v1(&descriptor, error, sizeof(error)) != MLN_PLUGIN_STATUS_OK) std::exit(2);
            if (mln_plugin_register_v1(&descriptor, error, sizeof(error)) != MLN_PLUGIN_STATUS_ALREADY_REGISTERED)
                std::exit(3);
            auto after = parseCircle();
            if (!after || (*after)->getTypeInfo() == builtinIdentity) std::exit(4);
            auto* manager = LayerManager::get();
            if (!manager->createRenderLayer((*before)->baseImpl) || !manager->createRenderLayer((*after)->baseImpl))
                std::exit(5);
            style::CircleLayer direct("direct", "points");
            if (direct.getTypeInfo() != builtinIdentity || !manager->createRenderLayer(direct.baseImpl)) std::exit(6);
            descriptor.plugin_id = {"test.conflict", 13};
            if (mln_plugin_register_v1(&descriptor, error, sizeof(error)) != MLN_PLUGIN_STATUS_CONFLICT) std::exit(7);
            std::exit(0);
        }()),
        ::testing::ExitedWithCode(0),
        "");
}

TEST(PluginRegistry, QueryBoundsIncludeEveryDrawable) {
    auto descriptor = *testDescriptor();
    auto layer = descriptor.layer_types[0];
    auto shader = layer.shaders[0];
    auto uniform = shader.uniform_blocks[0];
    uniform.byte_size = 32;
    mln_plugin_property_descriptor_v1 properties[] = {layer.properties[0], layer.properties[0]};
    properties[1].name = {"test-translate", 14};
    properties[1].type = MLN_PLUGIN_VALUE_FLOAT2;
    properties[1].default_value.type = MLN_PLUGIN_VALUE_FLOAT2;
    properties[1].default_value.data.float2_value = {0, 0};
    const mln_plugin_shader_attribute_v1 attributes[] = {
        shader.attributes[0],
        shader.attributes[1],
        {sizeof(mln_plugin_shader_attribute_v1), 2, 2, {"a_translate", 11}, MLN_PLUGIN_VERTEX_FLOAT_X4}};
    const mln_plugin_shader_property_binding_v1 bindings[] = {shader.property_bindings[0],
                                                              {sizeof(mln_plugin_shader_property_binding_v1),
                                                               {"test-translate", 14},
                                                               MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2,
                                                               0,
                                                               8,
                                                               2,
                                                               2,
                                                               0,
                                                               16}};
    shader.attributes = attributes;
    shader.attribute_count = 3;
    shader.uniform_blocks = &uniform;
    shader.property_bindings = bindings;
    shader.property_binding_count = 2;
    layer.layer_type = {"test.query-bounds", 17};
    layer.properties = properties;
    layer.shaders = &shader;
    layer.get_query_radius = [](const mln_plugin_property_statistics_v1* statistics,
                                size_t count,
                                const mln_plugin_property_value_v1*,
                                size_t) {
        EXPECT_EQ(2u, count);
        float radius = 0;
        for (size_t i = 0; i < count; ++i) {
            const auto& stat = statistics[i];
            if (stat.maximum.type == MLN_PLUGIN_VALUE_FLOAT) {
                EXPECT_FLOAT_EQ(5, stat.minimum.data.float_value);
                EXPECT_FLOAT_EQ(100, stat.maximum.data.float_value);
                radius += stat.maximum.data.float_value;
            } else {
                EXPECT_FLOAT_EQ(-2, stat.minimum.data.float2_value.x);
                EXPECT_FLOAT_EQ(-30, stat.minimum.data.float2_value.y);
                EXPECT_FLOAT_EQ(20, stat.maximum.data.float2_value.x);
                EXPECT_FLOAT_EQ(10, stat.maximum.data.float2_value.y);
                radius += std::max({std::abs(stat.minimum.data.float2_value.x),
                                    std::abs(stat.minimum.data.float2_value.y),
                                    std::abs(stat.maximum.data.float2_value.x),
                                    std::abs(stat.maximum.data.float2_value.y)});
            }
        }
        return radius;
    };
    descriptor.plugin_id = layer.layer_type;
    descriptor.layer_types = &layer;
    char error[256]{};
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&descriptor, error, sizeof(error))) << error;
    const auto registration = plugin::PluginRegistry::get().findLayerType("test.query-bounds");
    ASSERT_TRUE(registration);
    style::conversion::Error conversionError;
    const auto parsed = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"bounds","type":"test.query-bounds","source":"points","paint":{
        "test-radius":["get","radius"],"test-translate":["get","offset"]}})",
        conversionError);
    ASSERT_TRUE(parsed) << conversionError.message;
    const auto& paint = static_cast<const style::PluginStyleLayer::Impl&>(*(*parsed)->baseImpl).pluginProperties;
    auto features = std::make_shared<mapbox::feature::feature_collection<int16_t>>();
    for (const bool large : {false, true}) {
        mapbox::feature::feature<int16_t> feature{mapbox::geometry::point<int16_t>(100, 100)};
        feature.properties = {{"radius", large ? 100.0 : 5.0},
                              {"offset", Value{std::vector<Value>{large ? 20.0 : -2.0, large ? -30.0 : 10.0}}}};
        features->push_back(std::move(feature));
    }
    const GeoJSONTileLayer source(features);
    for (const bool reverse : {false, true}) {
        SCOPED_TRACE(reverse ? "large first" : "small first");
        PluginBucket bucket(*registration);
        for (std::size_t i = 0; i < 2; ++i) {
            const std::vector<PluginFeatureVertexRange> ranges = {{reverse ? 1 - i : i, i + 1, 0, 1}};
            bucket.paintPropertyBinders["bounds"].emplace(
                i + 1,
                PluginPaintPropertyBinders(*registration,
                                           registration->shaders[0],
                                           i + 1,
                                           1,
                                           0,
                                           {0, 0, 0},
                                           paint,
                                           std::make_shared<const PluginFeatureData>(ranges, std::make_unique<GeoJSONTileLayer>(source))));
        }
        bucket.updateQueryRadius("bounds", paint, 0);
        EXPECT_FLOAT_EQ(130, bucket.queryRadii.at("bounds"));
    }
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
