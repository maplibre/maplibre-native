#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/plugin/plugin_api.h>
#include <mln/plugin/plugin_shader.hpp>
#include <mln/style/style.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/util/run_loop.hpp>
#include <gtest/gtest.h>

#include <array>
#include <cstring>

using namespace mln;

namespace {

mln_plugin_string view(const std::string& text) {
    return {text.data(), text.size()};
}

// Minimal geometry plugin for exercising the host, independent of example plugins.
// Two layer types deliberately share a shader ID, but draw in different colors
// and viewport halves. No network requests or image-baseline updates are needed.
std::array<unsigned, 2> uniformCalls{};
float sharedAlpha = 1;
bool failSharedUniform = false;

void registerTriangles(const std::string& pluginID,
                       bool packedColor = false,
                       bool withUniforms = false,
                       bool scopedUniforms = false,
                       bool stencilOverlapDedup = false,
                       mln_plugin_should_animate_fn shouldAnimate = nullptr) {
    static const float vertices[] = {-1, -1, 1, -1, 0, 1};
    static const uint16_t indices[] = {0, 1, 2};
    static const mln_plugin_vertex_stream_v1 stream = {
        sizeof(stream), 0, reinterpret_cast<const uint8_t*>(vertices), sizeof(vertices), 3, 2 * sizeof(float)};
    static const mln_plugin_attribute_binding_v1 binding = {sizeof(binding), 0, 0, 0};
    static const mln_plugin_segment_v1 segment = {sizeof(segment), 0, 0, 3, 3};
    static const mln_plugin_drawable_descriptor_v1 drawable = {
        sizeof(drawable), 1, {"main", 4}, &binding, 1, &segment, 1, MLN_PLUGIN_DRAWABLE_DEPTH_READ_ONLY, 0, 0, 0, 0};
    static const mln_plugin_feature_vertex_range_v1 range = {sizeof(range), 0, 1, 0, 3};
    const mln_plugin_shader_attribute_v1 attribute = {
        sizeof(attribute), 0, 0, {"a_pos", 5}, MLN_PLUGIN_VERTEX_FLOAT_X2};
    const mln_plugin_shader_attribute_v1 uniformAttributes[] = {
        attribute,
        {sizeof(attribute), 2, 1, {"a_radius", 8}, MLN_PLUGIN_VERTEX_FLOAT_X2},
        {sizeof(attribute), 3, 2, {"a_opacity", 9}, MLN_PLUGIN_VERTEX_FLOAT_X2}};
    const mln_plugin_uniform_block_descriptor_v1 uniform = {
        sizeof(uniform), 0, {"QueryUBO", 8}, 16, MLN_PLUGIN_SHADER_STAGE_VERTEX, MLN_PLUGIN_UNIFORM_DRAWABLE};
    const mln_plugin_uniform_block_descriptor_v1 scopedBlocks[] = {
        {sizeof(uniform), 0, {"TileUBO", 7}, 16, MLN_PLUGIN_SHADER_STAGE_VERTEX, MLN_PLUGIN_UNIFORM_DRAWABLE_ARRAY},
        {sizeof(uniform), 1, {"PaintUBO", 8}, 16, MLN_PLUGIN_SHADER_STAGE_FRAGMENT, MLN_PLUGIN_UNIFORM_LAYER}};
    const mln_plugin_shader_property_binding_v1 paintBindings[] = {
        {sizeof(paintBindings[0]), {"test-radius", 11}, MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 0, 2, 2, 0, 4},
        {sizeof(paintBindings[0]), {"test-opacity", 12}, MLN_PLUGIN_PROPERTY_ENCODING_FLOAT, 0, 8, 3, 3, 0, 12}};
    static const uint8_t colors[] = {128, 64, 0, 255, 128, 64, 0, 255, 128, 64, 0, 255};
    static const mln_plugin_vertex_stream_v1 colorStreams[] = {stream,
                                                               {sizeof(stream), 1, colors, sizeof(colors), 3, 4}};
    static const mln_plugin_attribute_binding_v1 colorBindings[] = {binding, {sizeof(binding), 1, 1, 0}};
    static const mln_plugin_drawable_descriptor_v1 colorDrawable = {sizeof(drawable),
                                                                    1,
                                                                    {"main", 4},
                                                                    colorBindings,
                                                                    2,
                                                                    &segment,
                                                                    1,
                                                                    MLN_PLUGIN_DRAWABLE_DEPTH_READ_ONLY,
                                                                    0,
                                                                    0,
                                                                    0,
                                                                    0};
    const mln_plugin_shader_attribute_v1 colorAttributes[] = {
        attribute, {sizeof(attribute), 1, 1, {"a_color", 7}, MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED}};
    std::array<std::string, 2> types = {pluginID + ".left", pluginID + ".right"};
    std::array<std::array<std::string, 5>, 2> code;
    std::array<std::array<mln_plugin_shader_source_v1, 3>, 2> sources;
    std::array<mln_plugin_shader_descriptor_v1, 2> shaders{};
    std::array<mln_plugin_layer_type_v1, 2> layers{};
    mln_plugin_property_descriptor_v1 radius{};
    radius.struct_size = sizeof(radius);
    radius.name = {"test-radius", 11};
    radius.type = MLN_PLUGIN_VALUE_FLOAT;
    radius.expression_capabilities = MLN_PLUGIN_EXPRESSION_FEATURE | MLN_PLUGIN_EXPRESSION_CAMERA;
    radius.default_value = {sizeof(mln_plugin_value), MLN_PLUGIN_VALUE_FLOAT, {.float_value = 1}};
    auto opacity = radius;
    opacity.name = {"test-opacity", 12};
    const mln_plugin_property_descriptor_v1 properties[] = {radius, opacity};
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
        if (withUniforms) {
            const std::string constants = "const float offset = " + offset + ";const vec4 color = vec4(" + color +
                                          ");\n";
            code[i][0] = constants + R"GLSL(
in vec2 a_pos;
#if !MLN_PLUGIN_PROPERTY_TEST_RADIUS_IS_UNIFORM
layout(location=1) in vec2 a_radius;
#endif
#if !MLN_PLUGIN_PROPERTY_TEST_OPACITY_IS_UNIFORM
layout(location=2) in vec2 a_opacity;
#endif
layout(std140) uniform QueryUBO { float u_radius; float u_radius_t; float u_opacity; float u_opacity_t; };
out vec4 v_color;
void main() {
    float radius = u_radius;
    float opacity = u_opacity;
#if !MLN_PLUGIN_PROPERTY_TEST_RADIUS_IS_UNIFORM
    radius = mix(a_radius.x, a_radius.y, u_radius_t);
#endif
#if !MLN_PLUGIN_PROPERTY_TEST_OPACITY_IS_UNIFORM
    opacity = mix(a_opacity.x, a_opacity.y, u_opacity_t);
#endif
    gl_Position = vec4((a_pos.x + offset)*0.5, a_pos.y, radius*0.00001, 1);
    v_color = color*opacity;
}
)GLSL";
            code[i][1] = "in vec4 v_color;void main(){fragColor=v_color;}";
            code[i][2] = constants + R"GLSL(
layout(location=0) in vec2 a_pos;
#if !MLN_PLUGIN_PROPERTY_TEST_RADIUS_IS_UNIFORM
layout(location=1) in vec2 a_radius;
#endif
#if !MLN_PLUGIN_PROPERTY_TEST_OPACITY_IS_UNIFORM
layout(location=2) in vec2 a_opacity;
#endif
layout(set=DRAWABLE_UBO_SET_INDEX,binding=MLN_PLUGIN_UNIFORM_0_BINDING,std140)
uniform QueryUBO { float u_radius; float u_radius_t; float u_opacity; float u_opacity_t; };
layout(location=0) out vec4 v_color;
void main() {
    float radius = u_radius;
    float opacity = u_opacity;
#if !MLN_PLUGIN_PROPERTY_TEST_RADIUS_IS_UNIFORM
    radius = mix(a_radius.x, a_radius.y, u_radius_t);
#endif
#if !MLN_PLUGIN_PROPERTY_TEST_OPACITY_IS_UNIFORM
    opacity = mix(a_opacity.x, a_opacity.y, u_opacity_t);
#endif
    gl_Position = vec4((a_pos.x + offset)*0.5, a_pos.y, radius*0.00001, 1);
    applySurfaceTransform();
    v_color = color*opacity;
}
)GLSL";
            code[i][3] =
                "layout(location=0) in vec4 v_color;layout(location=0) out vec4 fragColor;"
                "void main(){fragColor=v_color;}";
            code[i][4] = "constant float offset = " + offset + ";constant float4 color = float4(" + color + ");\n" +
                         R"MSL(
struct Input {
    float2 a_pos [[attribute(0)]];
#if !MLN_PLUGIN_PROPERTY_TEST_RADIUS_IS_UNIFORM
    float2 a_radius [[attribute(1)]];
#endif
#if !MLN_PLUGIN_PROPERTY_TEST_OPACITY_IS_UNIFORM
    float2 a_opacity [[attribute(2)]];
#endif
};
struct QueryUBO { float radius; float radius_t; float opacity; float opacity_t; };
struct Output { float4 position [[position]]; float4 color; };
vertex Output triangleVertex(Input in [[stage_in]], constant QueryUBO& paint [[buffer(MLN_PLUGIN_UNIFORM_0_BINDING)]]) {
    float radius = paint.radius;
    float opacity = paint.opacity;
#if !MLN_PLUGIN_PROPERTY_TEST_RADIUS_IS_UNIFORM
    radius = mix(in.a_radius.x, in.a_radius.y, paint.radius_t);
#endif
#if !MLN_PLUGIN_PROPERTY_TEST_OPACITY_IS_UNIFORM
    opacity = mix(in.a_opacity.x, in.a_opacity.y, paint.opacity_t);
#endif
    return {float4((in.a_pos.x + offset)*0.5, in.a_pos.y, radius*0.00001, 1), color*opacity};
}
fragment half4 triangleFragment(Output in [[stage_in]]) { return half4(in.color); }
)MSL";
        }
        if (scopedUniforms) {
            // Use instance names to keep uniform members distinct across GL shader stages.
            const auto block = [](const char* name, const char* instance, unsigned id, bool vulkan) {
                return std::string("layout(std140") +
                       (vulkan ? ",set=DRAWABLE_UBO_SET_INDEX,binding=MLN_PLUGIN_UNIFORM_" + std::to_string(id) +
                                     "_BINDING"
                               : "") +
                       ") uniform " + name + "{vec4 value;}" + instance + ";";
            };
            code[i][0] = "in vec2 a_pos;" + block("TileUBO", "tile", 0, false) + body + "gl_Position.z=tile.value.x;}";
            code[i][1] = block("PaintUBO", "paint", 1, false) + "void main(){fragColor=vec4(" + color +
                         ")*paint.value.x;}";
            code[i][2] = "layout(location=0) in vec2 a_pos;" + block("TileUBO", "tile", 0, true) + body +
                         "gl_Position.z=tile.value.x;applySurfaceTransform();}";
            code[i][3] = "layout(location=0) out vec4 fragColor;" + block("PaintUBO", "paint", 1, true) +
                         "void main(){fragColor=vec4(" + color + ")*paint.value.x;}";
            code[i][4] =
                "struct Input{float2 a_pos [[attribute(0)]];};struct UBO{float4 value;};"
                "vertex float4 triangleVertex(Input in [[stage_in]],"
                "constant UBO* tiles [[buffer(MLN_PLUGIN_UNIFORM_0_BINDING)]],"
                "constant uint& index [[buffer(MLN_PLUGIN_DRAWABLE_INDEX_BINDING)]])"
                "{return float4((in.a_pos.x" +
                offset +
                ")*0.5,in.a_pos.y,tiles[index].value.x,1);}"
                "fragment half4 triangleFragment(constant UBO& paint [[buffer(MLN_PLUGIN_UNIFORM_1_BINDING)]])"
                "{return half4(" +
                color + ")*half(paint.value.x);}";
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
                      withUniforms ? uniformAttributes : (packedColor ? colorAttributes : &attribute),
                      withUniforms ? 3u : (packedColor ? 2u : 1u),
                      scopedUniforms ? scopedBlocks : (withUniforms ? &uniform : nullptr),
                      scopedUniforms ? 2u : (withUniforms ? 1u : 0u),
                      withUniforms ? paintBindings : nullptr,
                      withUniforms ? 2u : 0u};
        auto& layer = layers[i];
        layer.struct_size = sizeof(layer);
        layer.layer_type = view(types[i]);
        layer.backend_mask = MLN_PLUGIN_BACKEND_OPENGL | MLN_PLUGIN_BACKEND_VULKAN | MLN_PLUGIN_BACKEND_METAL;
        layer.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POINT;
        layer.shaders = &shaders[i];
        layer.shader_count = 1;
        layer.properties = withUniforms ? properties : nullptr;
        layer.property_count = withUniforms ? 2u : 0u;
        layer.enable_stencil_overlap_dedup = stencilOverlapDedup;
        layer.should_animate = shouldAnimate;
        layer.update_uniform_block = [](const mln_plugin_uniform_context_v1*, uint32_t, uint8_t*, size_t) {
            return MLN_PLUGIN_STATUS_OK;
        };
        if (scopedUniforms) {
            layer.update_uniform_block =
                [](const mln_plugin_uniform_context_v1* context, uint32_t id, uint8_t* bytes, size_t size) {
                    EXPECT_EQ(16u, size);
                    ++uniformCalls.at(id);
                    if (failSharedUniform) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
                    if (id == 1) {
                        EXPECT_EQ(0, context->pixels_to_tile_units);
                        for (unsigned i = 0; i < 16; ++i) EXPECT_EQ(i % 5 == 0 ? 1 : 0, context->tile_matrix[i]);
                        std::memcpy(bytes, &sharedAlpha, sizeof(sharedAlpha));
                    }
                    return MLN_PLUGIN_STATUS_OK;
                };
        }
        layer.create_layout = [](const mln_plugin_layout_context_v1*, void** instance) {
            *instance = new (std::nothrow) int(0);
            return *instance ? MLN_PLUGIN_STATUS_OK : MLN_PLUGIN_STATUS_CALLBACK_ERROR;
        };
        layer.layout_feature = [](void*, const mln_plugin_feature_v1*) {
            return MLN_PLUGIN_STATUS_OK;
        };
        layer.finish_layout = [](void*, mln_plugin_bucket_v1* output) {
            *output = {sizeof(*output), &stream, 1, indices, 3, &drawable, 1, 0, &range, 1};
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

TEST(PluginRendering, StencilOverlapDedupGeneratesStencilUpdates) {
    ASSERT_NO_FATAL_FAILURE(registerTriangles("test.stencil-dedup",
                                              /*packedColor=*/false,
                                              /*withUniforms=*/false,
                                              /*scopedUniforms=*/false,
                                              /*stencilOverlapDedup=*/true));
    RenderTest test;
    test.expectTriangles("test.stencil-dedup");
    const auto result = test.frontend.render(test.map);
    EXPECT_GT(result.stats.stencilUpdates, 0u);
}

TEST(PluginRendering, CameraPaintRefreshesAfterPropertyAndZoomChanges) {
    static float radius;
    const auto recordRadius = [](const mln_plugin_property_value_v1* properties, size_t) -> uint8_t {
        radius = properties[0].value.data.float_value;
        return false;
    };
    ASSERT_NO_FATAL_FAILURE(registerTriangles("test.camera-paint", false, true, false, false, recordRadius));
    RenderTest test;
    test.map.getStyle().loadJSON(R"({"version":8,"sources":{"points":{"type":"geojson","maxzoom":0,"data":{
        "type":"Point","coordinates":[0,0]}}},"layers":[{"id":"left","type":"test.camera-paint.left",
        "source":"points","paint":{"test-radius":0.25}}]})");
    const auto expectRadius = [&](double zoom, float expected) {
        test.map.jumpTo(CameraOptions().withZoom(zoom));
        test.frontend.render(test.map);
        EXPECT_FLOAT_EQ(expected, radius);
    };
    expectRadius(0, 0.25f);
    JSDocument value;
    value.Parse(R"(["interpolate",["linear"],["zoom"],0,1,1,0.5])");
    ASSERT_FALSE(test.map.getStyle().getLayer("left")->setProperty(
        "test-radius", style::conversion::Convertible(static_cast<const JSValue*>(&value))));
    expectRadius(0, 1);
    expectRadius(1, 0.5f);
}

TEST(PluginRendering, UniformPaintBetweenVertexAttributes) {
    ASSERT_NO_FATAL_FAILURE(registerTriangles("test.mixed-paint", /*packedColor=*/false, /*withUniforms=*/true));
    RenderTest test;
    test.map.getStyle().loadJSON(R"({"version":8,"sources":{"points":{"type":"geojson","data":{
        "type":"Feature","properties":{"radius":0.5,"opacity":0.25},
        "geometry":{"type":"Point","coordinates":[0,0]}}}},"layers":[
        {"id":"left","type":"test.mixed-paint.left","source":"points"}]})");
    // A uniform radius at location 1 leaves a hole before data-driven opacity
    // at location 2. Switching either property must preserve the opacity input.
    struct Paint {
        const char* radius;
        const char* opacity;
        int expected;
    };
    const Paint cases[] = {{"1", R"(["get","opacity"])", 64},
                           {R"(["get","radius"])", R"(["get","opacity"])", 64},
                           {"0.25", "0.5", 128},
                           {"0.25", R"(["get","opacity"])", 64}};
    for (const auto& paint : cases) {
        SCOPED_TRACE(std::string(paint.radius) + ", " + paint.opacity);
        for (const auto& [name, json] :
             {std::pair{"test-radius", paint.radius}, std::pair{"test-opacity", paint.opacity}}) {
            JSDocument value;
            value.Parse(json);
            ASSERT_FALSE(test.map.getStyle().getLayer("left")->setProperty(
                name, style::conversion::Convertible(static_cast<const JSValue*>(&value))));
        }
        const auto result = test.frontend.render(test.map);
        const auto* pixel = result.image.data.get() + (32 * 64 + 16) * 4;
        EXPECT_NEAR(paint.expected, pixel[0], 1);
        EXPECT_EQ(0, pixel[1]);
        EXPECT_EQ(0, pixel[2]);
        EXPECT_NEAR(paint.expected, pixel[3], 1);
    }
}

TEST(PluginRendering, UnchangedUniformUploadsAreSkippedButPaintChangesUpload) {
    ASSERT_NO_FATAL_FAILURE(registerTriangles("test.cached-uniforms", false, true));
    RenderTest test;
    test.map.getStyle().loadJSON(triangleStyle("test.cached-uniforms"));
    const auto first = test.frontend.render(test.map).stats;
    ASSERT_GT(first.numDrawCalls, 0u);
    const auto warm = test.frontend.render(test.map).stats;
    const auto unchangedBytes = warm.uniformUpdateBytes - first.uniformUpdateBytes;
    JSDocument value;
    value.SetDouble(23);
    ASSERT_FALSE(test.map.getStyle().getLayer("left")->setProperty(
        "test-radius", style::conversion::Convertible(static_cast<const JSValue*>(&value))));
    const auto changed = test.frontend.render(test.map).stats;
    EXPECT_GT(changed.uniformUpdateBytes - warm.uniformUpdateBytes, unchangedBytes);
    const auto stable = test.frontend.render(test.map).stats;
    EXPECT_EQ(unchangedBytes, stable.uniformUpdateBytes - changed.uniformUpdateBytes);
}

TEST(PluginRendering, ShaderIdentityHasUnambiguousComponents) {
    EXPECT_NE(plugin::shaderGroupName("a/b", "c", "d"), plugin::shaderGroupName("a", "b/c", "d"));
    EXPECT_NE(plugin::shaderGroupName("a", "b/c", "d"), plugin::shaderGroupName("a", "b", "c/d"));
    EXPECT_NE(plugin::shaderGroupName("a", "b", "main"), plugin::shaderGroupName("a", "c", "main"));
}

TEST(PluginRendering, SharedUniformsRunAtDeclaredScopeAndUpdateWithoutGeometryChurn) {
    ASSERT_NO_FATAL_FAILURE(registerTriangles("test.shared-uniforms", false, false, true));
    RenderTest test;
    test.map.jumpTo(CameraOptions().withZoom(2)); // The point lies on four tile boundaries.
    test.expectTriangles("test.shared-uniforms");
    const auto first = test.frontend.render(test.map);
    uniformCalls = {};
    const auto warm = test.frontend.render(test.map);
    EXPECT_EQ(2u, uniformCalls[1]); // One callback for each layer, not each tile.
    EXPECT_GT(uniformCalls[0], uniformCalls[1]);
    EXPECT_EQ(8u, uniformCalls[0]); // Two layers, four tile drawables each.
    EXPECT_EQ(first.stats.totalBuffers, warm.stats.totalBuffers);
    EXPECT_EQ(first.stats.vertexUpdateBytes, warm.stats.vertexUpdateBytes);
    EXPECT_EQ(first.stats.indexUpdateBytes, warm.stats.indexUpdateBytes);
    sharedAlpha = 0.25f; // A stateful layer callback must still execute every frame.
    const auto changed = test.frontend.render(test.map);
    EXPECT_NE(0, std::memcmp(warm.image.data.get(), changed.image.data.get(), warm.image.bytes()));
    EXPECT_EQ(warm.stats.totalBuffers, changed.stats.totalBuffers);
    EXPECT_GT(changed.stats.uniformUpdateBytes - warm.stats.uniformUpdateBytes,
              warm.stats.uniformUpdateBytes - first.stats.uniformUpdateBytes);
    sharedAlpha = 1;
    test.map.jumpTo(CameraOptions().withZoom(0)); // Shrink the batch, then grow it again.
    test.frontend.render(test.map);
    uniformCalls = {};
    test.frontend.render(test.map);
    EXPECT_EQ(2u, uniformCalls[0]);
    EXPECT_EQ(2u, uniformCalls[1]);
    test.map.jumpTo(CameraOptions().withZoom(2));
    test.frontend.render(test.map);
    uniformCalls = {};
    const auto restored = test.frontend.render(test.map);
    EXPECT_EQ(8u, uniformCalls[0]);
    EXPECT_EQ(2u, uniformCalls[1]);
    EXPECT_EQ(warm.image, restored.image);
    failSharedUniform = true;
    const auto failed = test.frontend.render(test.map);
    EXPECT_NE(warm.image, failed.image);
    failSharedUniform = false;
    const auto recovered = test.frontend.render(test.map);
    EXPECT_EQ(warm.image, recovered.image);
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
