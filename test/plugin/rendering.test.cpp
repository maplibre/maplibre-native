#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/plugin/plugin_api.h>
#include <mln/plugin/plugin_shader.hpp>
#include <mln/style/style.hpp>
#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/util/run_loop.hpp>
#include <gtest/gtest.h>

#include <array>

using namespace mln;

namespace {

mln_plugin_string view(const std::string& text) {
    return {text.data(), text.size()};
}

// Minimal geometry plugin for exercising the host, independent of example plugins.
// Two layer types deliberately share a shader ID, but draw in different colors
// and viewport halves. No network requests or image-baseline updates are needed.
void registerTriangles(const std::string& pluginID, bool packedColor = false) {
    static const float vertices[] = {-1, -1, 1, -1, 0, 1};
    static const uint16_t indices[] = {0, 1, 2};
    static const mln_plugin_vertex_stream_v1 stream = {
        sizeof(stream), 0, reinterpret_cast<const uint8_t*>(vertices), sizeof(vertices), 3, 2 * sizeof(float)};
    static const mln_plugin_attribute_binding_v1 binding = {sizeof(binding), 0, 0, 0, MLN_PLUGIN_VERTEX_FLOAT_X2};
    static const mln_plugin_segment_v1 segment = {sizeof(segment), 0, 0, 3, 3, 0};
    static const mln_plugin_drawable_descriptor_v1 drawable = {
        sizeof(drawable), 1, {"main", 4}, &binding, 1, &segment, 1};
    const mln_plugin_shader_attribute_v1 attribute = {
        sizeof(attribute), 0, 0, {"a_pos", 5}, MLN_PLUGIN_VERTEX_FLOAT_X2};
    static const uint8_t colors[] = {128, 64, 0, 255, 128, 64, 0, 255, 128, 64, 0, 255};
    static const mln_plugin_vertex_stream_v1 colorStreams[] = {stream,
                                                               {sizeof(stream), 1, colors, sizeof(colors), 3, 4}};
    static const mln_plugin_attribute_binding_v1 colorBindings[] = {
        binding, {sizeof(binding), 1, 1, 0, MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED}};
    static const mln_plugin_drawable_descriptor_v1 colorDrawable = {
        sizeof(drawable), 1, {"main", 4}, colorBindings, 2, &segment, 1};
    const mln_plugin_shader_attribute_v1 colorAttributes[] = {
        attribute, {sizeof(attribute), 1, 1, {"a_color", 7}, MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED}};
    std::array<std::string, 2> types = {pluginID + ".left", pluginID + ".right"};
    std::array<std::array<std::string, 5>, 2> code;
    std::array<std::array<mln_plugin_shader_source_v1, 3>, 2> sources;
    std::array<mln_plugin_shader_descriptor_v1, 2> shaders{};
    std::array<mln_plugin_layer_type_v1, 2> layers{};
    for (std::size_t i = 0; i < layers.size(); ++i) {
        const std::string offset = i == 0 ? "-1.0" : "+1.0";
        const std::string color = i == 0 ? "1,0,0,1" : "0,1,0,1";
        const std::string body = "void main(){gl_Position=vec4((a_pos.x" + offset + ")*0.5,a_pos.y,0,1);";
        code[i] = {"in vec2 a_pos;" + body + "}",
                   "void main(){fragColor=vec4(" + color + ");}",
                   "layout(location=0) in vec2 a_pos;" + body + "applySurfaceTransform();}",
                   "layout(location=0) out vec4 fragColor;void main(){fragColor=vec4(" + color + ");}",
                   "struct Input{float2 a_pos [[attribute(0)]];};"
                   "vertex float4 triangleVertex(Input in [[stage_in]]) {return float4((in.a_pos.x" +
                       offset + ")*0.5,in.a_pos.y,0,1);}fragment half4 triangleFragment(){return half4(" + color +
                       ");}"};
        if (packedColor) {
            code[i] = {"in vec2 a_pos;in vec4 a_color;out vec4 v_color;" + body + "v_color=a_color;}",
                       "in vec4 v_color;void main(){fragColor=v_color;}",
                       "layout(location=0) in vec2 a_pos;layout(location=1) in vec4 a_color;"
                       "layout(location=0) out vec4 v_color;" +
                           body + "v_color=a_color;applySurfaceTransform();}",
                       "layout(location=0) in vec4 v_color;layout(location=0) out vec4 fragColor;"
                       "void main(){fragColor=v_color;}",
                       "struct Input{float2 a_pos [[attribute(0)]];float4 a_color [[attribute(1)]];};"
                       "struct Output{float4 position [[position]];float4 color;};"
                       "vertex Output triangleVertex(Input in [[stage_in]]){return {float4((in.a_pos.x" +
                           offset +
                           ")*0.5,in.a_pos.y,0,1),in.a_color};}"
                           "fragment half4 triangleFragment(Output in [[stage_in]]){return half4(in.color);}"};
        }
        sources[i] = {{{sizeof(mln_plugin_shader_source_v1),
                        MLN_PLUGIN_BACKEND_OPENGL,
                        view(code[i][0]),
                        view(code[i][1]),
                        {},
                        {}},
                       {sizeof(mln_plugin_shader_source_v1),
                        MLN_PLUGIN_BACKEND_VULKAN,
                        view(code[i][2]),
                        view(code[i][3]),
                        {},
                        {}},
                       {sizeof(mln_plugin_shader_source_v1),
                        MLN_PLUGIN_BACKEND_METAL,
                        view(code[i][4]),
                        {},
                        {"triangleVertex", 14},
                        {"triangleFragment", 16}}}};
        shaders[i] = {sizeof(mln_plugin_shader_descriptor_v1),
                      {"main", 4},
                      sources[i].data(),
                      3,
                      packedColor ? colorAttributes : &attribute,
                      packedColor ? 2u : 1u,
                      nullptr,
                      0,
                      nullptr,
                      0};
        auto& layer = layers[i];
        layer.struct_size = sizeof(layer);
        layer.layer_type = view(types[i]);
        layer.backend_mask = MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_VULKAN | MLN_PLUGIN_BACKEND_METAL;
        layer.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POINT;
        layer.shaders = &shaders[i];
        layer.shader_count = 1;
        layer.create_layout = [](const mln_plugin_layout_context_v1*, void** instance) {
            *instance = new (std::nothrow) int(0);
            return *instance ? MLN_PLUGIN_STATUS_OK : MLN_PLUGIN_STATUS_CALLBACK_ERROR;
        };
        layer.layout_feature = [](void*, const mln_plugin_feature_v1*) {
            return MLN_PLUGIN_STATUS_OK;
        };
        layer.finish_layout = [](void*, mln_plugin_bucket_v1* output) {
            *output = {sizeof(*output), &stream, 1, indices, 3, &drawable, 1, 0, nullptr, 0};
            return MLN_PLUGIN_STATUS_OK;
        };
        if (packedColor) {
            layer.finish_layout = [](void*, mln_plugin_bucket_v1* output) {
                *output = {sizeof(*output), colorStreams, 2, indices, 3, &colorDrawable, 1, 0, nullptr, 0};
                return MLN_PLUGIN_STATUS_OK;
            };
        }
        layer.destroy_layout = [](void* instance) {
            delete static_cast<int*>(instance);
        };
    }
    const mln_plugin_descriptor_v1 descriptor = {
        sizeof(descriptor), MLN_PLUGIN_ABI_VERSION_1, view(pluginID), {"1", 1}, 1, 1, layers.data(), layers.size()};
    char error[256]{};
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_plugin_register_v1(&descriptor, error, sizeof(error))) << error;
}

std::string triangleStyle(const std::string& pluginID) {
    return R"({"version":8,"sources":{"points":{"type":"geojson","data":{
        "type":"FeatureCollection","features":[{"type":"Feature","properties":{},
        "geometry":{"type":"Point","coordinates":[0,0]}}]}}},"layers":[
        {"id":"left","type":")" +
           pluginID + R"(.left","source":"points"},
        {"id":"right","type":")" +
           pluginID + R"(.right","source":"points"}]})";
}

struct RenderTest {
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>();
    HeadlessFrontend frontend{{64, 64}, 1};
    MapAdapter map{frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())};

    void expectTriangles(const std::string& pluginID) {
        map.getStyle().loadJSON(triangleStyle(pluginID));
        const auto result = frontend.render(map);
        const auto* left = result.image.data.get() + (32 * 64 + 16) * 4;
        const auto* right = result.image.data.get() + (32 * 64 + 48) * 4;
        EXPECT_EQ(255, left[0]);
        EXPECT_EQ(0, left[1]);
        EXPECT_EQ(255, left[3]);
        EXPECT_EQ(0, right[0]);
        EXPECT_EQ(255, right[1]);
        EXPECT_EQ(255, right[3]);
    }
};

TEST(PluginRendering, LayerLocalShaderIDsProduceDifferentPrograms) {
    registerTriangles("test.shader-identity");
    RenderTest test;
    test.expectTriangles("test.shader-identity");
}

TEST(PluginRendering, DoesNotGenerateUnusedStencilMasks) {
    ASSERT_NO_FATAL_FAILURE(registerTriangles("test.no-stencil"));
    RenderTest test;
    test.expectTriangles("test.no-stencil");
    const auto result = test.frontend.render(test.map);
    EXPECT_EQ(0, result.stats.stencilUpdates);
}

TEST(PluginRendering, ShaderIdentityHasUnambiguousComponents) {
    EXPECT_NE(plugin::shaderGroupName("a/b", "c", "d"), plugin::shaderGroupName("a", "b/c", "d"));
    EXPECT_NE(plugin::shaderGroupName("a", "b/c", "d"), plugin::shaderGroupName("a", "b", "c/d"));
    EXPECT_NE(plugin::shaderGroupName("a", "b", "main"), plugin::shaderGroupName("a", "c", "main"));
}

TEST(PluginRendering, RegistrationAfterRendererInitialization) {
    {
        RenderTest test;
        test.map.getStyle().loadJSON(R"({"version":8,"sources":{},"layers":[]})");
        test.frontend.render(test.map);
        // The public contract requires registration before the dependent style,
        // not before constructing or rendering any map in the process.
        registerTriangles("test.late-registration");
        test.expectTriangles("test.late-registration");
    }
    RenderTest newMap;
    newMap.expectTriangles("test.late-registration");
}

TEST(PluginRendering, NormalizedByteColorAttributes) {
    registerTriangles("test.normalized-color", true);
    RenderTest test;
    test.map.getStyle().loadJSON(triangleStyle("test.normalized-color"));
    const auto result = test.frontend.render(test.map);
    for (const auto x : {16, 48}) {
        const auto* pixel = result.image.data.get() + (32 * 64 + x) * 4;
        EXPECT_NEAR(128, pixel[0], 1);
        EXPECT_NEAR(64, pixel[1], 1);
        EXPECT_EQ(0, pixel[2]);
        EXPECT_EQ(255, pixel[3]);
    }
}

} // namespace
