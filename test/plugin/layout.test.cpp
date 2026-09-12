#include <mln/geometry/feature_index.hpp>
#include <mln/layout/plugin_layout.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/tile/geojson_tile_data.hpp>
#include <gtest/gtest.h>

#include <array>
#include <cstring>
#include <limits>

using namespace mln;

namespace {

enum class Output {
    Valid,
    CreateErrorWithHandle,
    CreateErrorWithoutHandle,
    CreateSuccessWithoutHandle,
    FeatureError,
    FinishError,
    ShortBucket,
    ShortStream,
    ShortDrawable,
    ShortAttribute,
    ShortSegment,
    ShortRange,
    MissingStreams,
    MissingIndices,
    MissingDrawables,
    MissingAttributes,
    MissingSegments,
    MissingRanges,
    MissingVertexData,
    WrongByteCount,
    BadIndex,
    BadVertexRange,
    BadQueryRadius,
    Empty,
};

struct LayoutData {
    std::array<int16_t, 6> vertices = {10, 20, 30, 40, 50, 60};
    std::array<uint16_t, 3> indices = {0, 1, 2};
    std::string shaderID = "test-shader";
    mln_plugin_vertex_stream_v1 stream = {
        sizeof(stream), 0, reinterpret_cast<const uint8_t*>(vertices.data()), sizeof(vertices), 3, 2 * sizeof(int16_t)};
    mln_plugin_attribute_binding_v1 attribute = {sizeof(attribute), 0, 0, 0, MLN_PLUGIN_VERTEX_INT16_X2};
    mln_plugin_segment_v1 segment = {sizeof(segment), 0, 0, 3, 3, 0};
    mln_plugin_drawable_descriptor_v1 drawable = {
        sizeof(drawable), 7, {shaderID.data(), shaderID.size()}, &attribute, 1, &segment, 1};
    mln_plugin_feature_vertex_range_v1 range = {sizeof(range), 0, 7, 0, 3};
    // An independently allocated prefix makes premature reads visible to ASan.
    std::unique_ptr<uint32_t> shortSize = std::make_unique<uint32_t>(sizeof(uint32_t));
};

struct State {
    Output output = Output::Valid;
    unsigned creates = 0, features = 0, finishes = 0, destroys = 0;
    // Fallback cleanup also keeps the pre-fix failure test from leaking.
    std::unique_ptr<LayoutData> instance;
};
State* active = nullptr;

mln_plugin_status create(const mln_plugin_layout_context_v1*, void** instance) {
    ++active->creates;
    EXPECT_EQ(nullptr, *instance);
    if (active->output == Output::CreateErrorWithoutHandle) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    if (active->output == Output::CreateSuccessWithoutHandle) return MLN_PLUGIN_STATUS_OK;
    active->instance = std::make_unique<LayoutData>();
    *instance = active->instance.get();
    return active->output == Output::CreateErrorWithHandle ? MLN_PLUGIN_STATUS_CALLBACK_ERROR : MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status feature(void* instance, const mln_plugin_feature_v1* input) {
    ++active->features;
    EXPECT_EQ(active->instance.get(), instance);
    EXPECT_EQ(1u, input->point_count);
    EXPECT_EQ(1u, input->path_count);
    EXPECT_EQ(10, input->points[0].x);
    EXPECT_EQ(20, input->points[0].y);
    EXPECT_EQ(0u, input->path_offsets[0]);
    EXPECT_EQ(1u, input->path_offsets[1]);
    return active->output == Output::FeatureError ? MLN_PLUGIN_STATUS_CALLBACK_ERROR : MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status finish(void* instance, mln_plugin_bucket_v1* output) {
    ++active->finishes;
    EXPECT_EQ(active->instance.get(), instance);
    auto& data = *static_cast<LayoutData*>(instance);
    *output = {sizeof(*output),
               &data.stream,
               1,
               data.indices.data(),
               data.indices.size(),
               &data.drawable,
               1,
               12,
               &data.range,
               1};
    switch (active->output) {
        case Output::FinishError:
            return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
        case Output::ShortBucket:
            output->struct_size = sizeof(uint32_t);
            break;
        case Output::ShortStream:
            output->vertex_streams = reinterpret_cast<const mln_plugin_vertex_stream_v1*>(data.shortSize.get());
            break;
        case Output::ShortDrawable:
            output->drawables = reinterpret_cast<const mln_plugin_drawable_descriptor_v1*>(data.shortSize.get());
            break;
        case Output::ShortAttribute:
            data.drawable.attributes = reinterpret_cast<const mln_plugin_attribute_binding_v1*>(data.shortSize.get());
            break;
        case Output::ShortSegment:
            data.drawable.segments = reinterpret_cast<const mln_plugin_segment_v1*>(data.shortSize.get());
            break;
        case Output::ShortRange:
            output->feature_vertex_ranges = reinterpret_cast<const mln_plugin_feature_vertex_range_v1*>(
                data.shortSize.get());
            break;
        case Output::MissingStreams:
            output->vertex_streams = nullptr;
            break;
        case Output::MissingIndices:
            output->indices = nullptr;
            break;
        case Output::MissingDrawables:
            output->drawables = nullptr;
            break;
        case Output::MissingAttributes:
            data.drawable.attributes = nullptr;
            break;
        case Output::MissingSegments:
            data.drawable.segments = nullptr;
            break;
        case Output::MissingRanges:
            output->feature_vertex_ranges = nullptr;
            break;
        case Output::MissingVertexData:
            data.stream.data = nullptr;
            break;
        case Output::WrongByteCount:
            ++data.stream.data_size;
            break;
        case Output::BadIndex:
            data.indices[2] = 3;
            break;
        case Output::BadVertexRange:
            data.range.vertex_count = std::numeric_limits<uint32_t>::max();
            break;
        case Output::BadQueryRadius:
            output->query_radius = std::numeric_limits<float>::quiet_NaN();
            break;
        case Output::Empty:
            *output = {};
            output->struct_size = sizeof(*output);
            break;
        default:
            break;
    }
    return MLN_PLUGIN_STATUS_OK;
}

void destroy(void* instance) {
    ++active->destroys;
    EXPECT_NE(nullptr, instance);
    EXPECT_EQ(active->instance.get(), instance);
    active->instance.reset();
}

std::shared_ptr<PluginBucket> build(State& state) {
    struct Scope {
        explicit Scope(State& value) { active = &value; }
        ~Scope() { active = nullptr; }
    } scope(state);
    plugin::LayerType registration;
    registration.pluginID = "test.layout";
    registration.type = "test.layout.layer";
    registration.geometryTypeMask = MLN_PLUGIN_GEOMETRY_POINT;
    registration.createLayout = create;
    registration.layoutFeature = feature;
    registration.finishLayout = finish;
    registration.destroyLayout = destroy;
    registration.identity = std::make_shared<plugin::LayerTypeIdentity>(registration);
    plugin::ShaderDefinition shader;
    shader.id = "test-shader";
    shader.attributes.push_back({0, 0, "a_pos", MLN_PLUGIN_VERTEX_INT16_X2});
    registration.shaders.push_back(std::move(shader));
    auto impl = makeMutable<style::PluginStyleLayer::Impl>("test-layer", "points", registration);
    std::vector<Immutable<style::LayerProperties>> layers = {
        makeMutable<style::PluginStyleLayerProperties>(std::move(impl))};
    auto features = std::make_shared<mapbox::feature::feature_collection<int16_t>>();
    features->emplace_back(mapbox::geometry::point<int16_t>{10, 20});
    auto featureIndex = std::make_unique<FeatureIndex>(std::make_unique<GeoJSONTileData>(*features));
    const BucketParameters parameters{
        OverscaledTileID{0, 0, {0, 0, 0}}, MapMode::Static, 1, &registration.identity->info};
    PluginLayout layout(parameters, std::move(layers), std::make_unique<GeoJSONTileLayer>(features), registration);
    mln::unordered_map<std::string, LayerRenderData> renderData;
    layout.createBucket({}, featureIndex, renderData, false, false, parameters.tileID.canonical);
    return renderData.empty() ? nullptr : std::static_pointer_cast<PluginBucket>(renderData.at("test-layer").bucket);
}

TEST(PluginLayout, DestroysHandleReturnedByFailedCreate) {
    State state;
    state.output = Output::CreateErrorWithHandle;
    EXPECT_FALSE(build(state));
    EXPECT_EQ(1u, state.creates);
    EXPECT_EQ(0u, state.features);
    EXPECT_EQ(0u, state.finishes);
    EXPECT_EQ(1u, state.destroys);
}

TEST(PluginLayout, DoesNotDestroyNullHandles) {
    for (const auto output : {Output::CreateErrorWithoutHandle, Output::CreateSuccessWithoutHandle}) {
        State state;
        state.output = output;
        EXPECT_FALSE(build(state));
        EXPECT_EQ(1u, state.creates);
        EXPECT_EQ(0u, state.features);
        EXPECT_EQ(0u, state.finishes);
        EXPECT_EQ(0u, state.destroys);
    }
}

TEST(PluginLayout, DestroysExactlyOnceOnEveryEarlyReturn) {
    for (const auto output :
         {Output::FeatureError,    Output::FinishError,    Output::ShortBucket,       Output::ShortStream,
          Output::ShortDrawable,   Output::ShortAttribute, Output::ShortSegment,      Output::ShortRange,
          Output::MissingStreams,  Output::MissingIndices, Output::MissingDrawables,  Output::MissingAttributes,
          Output::MissingSegments, Output::MissingRanges,  Output::MissingVertexData, Output::WrongByteCount,
          Output::BadIndex,        Output::BadVertexRange, Output::BadQueryRadius,    Output::Empty}) {
        SCOPED_TRACE(static_cast<int>(output));
        State state;
        state.output = output;
        EXPECT_FALSE(build(state));
        EXPECT_EQ(1u, state.creates);
        EXPECT_EQ(1u, state.features);
        EXPECT_EQ(output == Output::FeatureError ? 0u : 1u, state.finishes);
        EXPECT_EQ(1u, state.destroys);
    }
}

TEST(PluginLayout, CopiesGeometryBeforeDestroyingLayout) {
    State state;
    const auto bucket = build(state);
    ASSERT_TRUE(bucket);
    EXPECT_EQ(1u, state.destroys);
    EXPECT_FALSE(state.instance);
    ASSERT_EQ(1u, bucket->vertexStreams.size());
    const auto& stream = *bucket->vertexStreams.at(0);
    EXPECT_EQ(3u, stream.getRawCount());
    EXPECT_EQ(2 * sizeof(int16_t), stream.getRawSize());
    const std::array<int16_t, 6> expected = {10, 20, 30, 40, 50, 60};
    EXPECT_EQ(0, std::memcmp(expected.data(), stream.getRawData(), sizeof(expected)));
    ASSERT_EQ(3u, bucket->indices->elements());
    EXPECT_EQ(2u, bucket->indices->vector()[2]);
    ASSERT_EQ(1u, bucket->drawables.size());
    const auto& drawable = bucket->drawables.front();
    EXPECT_EQ(7u, drawable.key);
    EXPECT_EQ("test-shader", drawable.shaderID);
    EXPECT_EQ(3u, drawable.vertexCount);
    ASSERT_EQ(1u, drawable.attributes.size());
    EXPECT_EQ(MLN_PLUGIN_VERTEX_INT16_X2, drawable.attributes.front().type);
    ASSERT_EQ(1u, drawable.segments.size());
    EXPECT_EQ(3u, drawable.segments.front().vertexLength);
    ASSERT_EQ(1u, bucket->featureVertexRanges.size());
    EXPECT_EQ(3u, bucket->featureVertexRanges.front().vertexCount);
    EXPECT_EQ(12.0f, bucket->queryRadius);
}

} // namespace
