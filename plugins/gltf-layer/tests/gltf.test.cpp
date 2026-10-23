#include "gltf_layer.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#define CHECK(condition)                                   \
    do {                                                   \
        if (!(condition)) {                                \
            std::cerr << __LINE__ << ": " #condition "\n"; \
            std::abort();                                  \
        }                                                  \
    } while (false)

namespace {
const mln_plugin_descriptor_v1* registered = nullptr;
mln_plugin_status capture(const mln_plugin_descriptor_v1* d, char*, size_t) {
    registered = d;
    return MLN_PLUGIN_STATUS_OK;
}

void append32(std::vector<uint8_t>& out, uint32_t value) {
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>(value >> (8 * i)));
}

// A binary glTF holding one triangle with a base color factor, built by hand so the test needs no asset.
std::vector<uint8_t> triangleGLB(const std::string& baseColor = "1,0.5,0.25,1") {
    std::string json =
        R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],)"
        R"("meshes":[{"primitives":[{"attributes":{"POSITION":0},"indices":1,"material":0}]}],)"
        R"("materials":[{"pbrMetallicRoughness":{"baseColorFactor":[)" +
        baseColor +
        R"(]}}],)"
        R"("buffers":[{"byteLength":44}],)"
        R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":36},{"buffer":0,"byteOffset":36,"byteLength":6}],)"
        R"("accessors":[{"bufferView":0,"componentType":5126,"count":3,"type":"VEC3","min":[0,0,0],"max":[1,1,0]},)"
        R"({"bufferView":1,"componentType":5123,"count":3,"type":"SCALAR"}]})";
    while (json.size() % 4 != 0) json.push_back(' ');

    std::vector<uint8_t> bin;
    const float positions[] = {0, 0, 0, 1, 0, 0, 0, 1, 0};
    bin.resize(sizeof(positions));
    std::memcpy(bin.data(), positions, sizeof(positions));
    for (uint16_t index : {0, 1, 2}) {
        bin.push_back(static_cast<uint8_t>(index));
        bin.push_back(0);
    }
    while (bin.size() % 4 != 0) bin.push_back(0);

    std::vector<uint8_t> glb;
    append32(glb, 0x46546C67); // "glTF"
    append32(glb, 2);
    append32(glb, static_cast<uint32_t>(12 + 8 + json.size() + 8 + bin.size()));
    append32(glb, static_cast<uint32_t>(json.size()));
    append32(glb, 0x4E4F534A); // "JSON"
    glb.insert(glb.end(), json.begin(), json.end());
    append32(glb, static_cast<uint32_t>(bin.size()));
    append32(glb, 0x004E4942); // "BIN\0"
    glb.insert(glb.end(), bin.begin(), bin.end());
    return glb;
}

mln_plugin_property_value_v1 modelID(void* model) {
    mln_plugin_property_value_v1 property{};
    property.struct_size = sizeof(property);
    property.name = {"model-id", 8};
    property.value.struct_size = sizeof(property.value);
    property.value.type = MLN_PLUGIN_VALUE_RESOURCE;
    property.value.data.resource_value = {{"model", 5}, model};
    return property;
}

// Lays out one feature per entry of `models`, all at `points`, with feature indices from `firstIndex`.
mln_plugin_bucket_v1 layOut(const mln_plugin_layer_type_v1& layer,
                            const std::vector<mln_plugin_tile_point_v1>& points,
                            const std::vector<void*>& models,
                            void*& instance,
                            uint64_t firstIndex = 3,
                            const mln_plugin_property_value_v1* occlusion = nullptr) {
    mln_plugin_layout_context_v1 context{};
    context.struct_size = sizeof(context);
    context.extent = 8192;
    context.properties = occlusion;
    context.property_count = occlusion ? 1 : 0;
    instance = nullptr;
    CHECK(layer.create_layout(&context, &instance) == MLN_PLUGIN_STATUS_OK);
    for (size_t i = 0; i < models.size(); ++i) {
        const auto property = modelID(models[i]);
        mln_plugin_feature_v1 feature{};
        feature.struct_size = sizeof(feature);
        feature.geometry_type = MLN_PLUGIN_GEOMETRY_POINT;
        feature.feature_index = firstIndex + i;
        feature.points = points.data();
        feature.point_count = points.size();
        feature.layout_properties = &property;
        feature.layout_property_count = 1;
        CHECK(layer.layout_feature(instance, &feature) == MLN_PLUGIN_STATUS_OK);
    }
    mln_plugin_bucket_v1 bucket{};
    bucket.struct_size = sizeof(bucket);
    CHECK(layer.finish_layout(instance, &bucket) == MLN_PLUGIN_STATUS_OK);
    return bucket;
}

void* createModel(const mln_plugin_layer_type_v1& layer, const std::vector<uint8_t>& glb) {
    void* handle = nullptr;
    CHECK(layer.create_resource(nullptr, {"test://model.glb", 16}, glb.data(), glb.size(), &handle) ==
          MLN_PLUGIN_STATUS_OK);
    CHECK(handle);
    return handle;
}
} // namespace

int main() {
    CHECK(mln_gltf_layer_register(nullptr, nullptr, 0) == MLN_PLUGIN_STATUS_NOT_FOUND);
    CHECK(mln_gltf_layer_register(capture, nullptr, 0) == MLN_PLUGIN_STATUS_OK);
    CHECK(registered && registered->layer_type_count == 1);
    const auto& layer = registered->layer_types[0];
    CHECK(std::string_view(layer.layer_type.data, layer.layer_type.size) == "gltf");
    CHECK(layer.backend_mask == MLN_PLUGIN_BACKEND_METAL && layer.enable_near_clipped_matrix);
    CHECK(layer.create_resource && layer.destroy_resource);

    // `models` maps names to .glb files and `model-id` names one of them; both are layout properties.
    CHECK(layer.properties[0].type == MLN_PLUGIN_VALUE_RESOURCES &&
          layer.properties[0].scope == MLN_PLUGIN_PROPERTY_LAYOUT);
    CHECK(layer.properties[1].type == MLN_PLUGIN_VALUE_RESOURCE &&
          layer.properties[1].scope == MLN_PLUGIN_PROPERTY_LAYOUT);
    CHECK(std::string_view(layer.properties[1].resources.data, layer.properties[1].resources.size) == "models");
    CHECK(std::string_view(layer.properties[2].name.data, layer.properties[2].name.size) == "model-occlusion" &&
          layer.properties[2].type == MLN_PLUGIN_VALUE_FLOAT && layer.properties[2].scope == MLN_PLUGIN_PROPERTY_LAYOUT);

    // Every paint property is bound to the shader, and the binding offsets stay inside the uniform block.
    const auto& shader = layer.shaders[0];
    CHECK(shader.property_binding_count + 3 == layer.property_count);
    CHECK(shader.attribute_count <= MLN_PLUGIN_MAX_VERTEX_ATTRIBUTES);
    std::set<uint32_t> locations;
    for (size_t i = 0; i < shader.attribute_count; ++i) {
        CHECK(locations.emplace(shader.attributes[i].location).second);
        CHECK(shader.attributes[i].attribute_id == shader.attributes[i].location);
    }
    const auto& uniform = shader.uniform_blocks[0];
    CHECK(uniform.byte_size % 16 == 0);
    for (size_t i = 0; i < shader.property_binding_count; ++i) {
        const auto& binding = shader.property_bindings[i];
        CHECK(binding.uniform_byte_offset + 4 <= uniform.byte_size);
        CHECK(binding.minimum_attribute_id < shader.attribute_count);
        CHECK(binding.interpolation_uniform_byte_offset + 4 <= uniform.byte_size);
        // The shader reads the factor as u.interpolation[i / 4][i % 4].
        CHECK((binding.interpolation_uniform_byte_offset - 128) / sizeof(float) == i);
    }

    // A malformed or empty asset is rejected and yields no handle.
    const uint8_t garbage[] = {1, 2, 3, 4, 5, 6, 7, 8};
    void* handle = &handle;
    CHECK(layer.create_resource(nullptr, {"test://bad.glb", 14}, garbage, sizeof(garbage), &handle) ==
          MLN_PLUGIN_STATUS_CALLBACK_ERROR);
    CHECK(handle == nullptr);
    CHECK(layer.create_resource(nullptr, {"test://empty.glb", 16}, nullptr, 0, &handle) ==
          MLN_PLUGIN_STATUS_CALLBACK_ERROR);
    CHECK(handle == nullptr);

    // A feature without a model (its name is not in `models`, or the model failed to load) is skipped.
    void* instance = nullptr;
    auto bucket = layOut(layer, {{10, 10}}, {nullptr}, instance);
    CHECK(bucket.drawable_count == 0 && bucket.index_count == 0);
    layer.destroy_layout(instance);

    const auto glb = triangleGLB();
    void* triangle = createModel(layer, glb);

    // Two in-tile points; the out-of-tile pair belongs to the neighbouring tiles.
    bucket = layOut(layer, {{0, 0}, {8191, 8191}, {8192, 0}, {-1, 0}}, {triangle}, instance);
    CHECK(bucket.vertex_stream_count == 1 && bucket.vertex_streams[0].vertex_count == 6);
    CHECK(bucket.vertex_streams[0].stride == 44);
    CHECK(bucket.index_count == 6 && bucket.drawable_count == 1);
    for (size_t i = 0; i < bucket.index_count; ++i) CHECK(bucket.indices[i] < 6);
    const auto& drawable = bucket.drawables[0];
    CHECK(drawable.depth_mode == MLN_PLUGIN_DRAWABLE_DEPTH_READ_WRITE && !drawable.enable_stencil_overlap);
    CHECK(bucket.feature_vertex_ranges[0].feature_index == 3);
    // The material's base color factor reaches the vertex stream.
    const float* firstVertex = reinterpret_cast<const float*>(bucket.vertex_streams[0].data + 28);
    CHECK(firstVertex[0] == 1.0f && firstVertex[1] == 0.5f && firstVertex[2] == 0.25f && firstVertex[3] == 1.0f);
    layer.destroy_layout(instance);

    // `model-occlusion` is a layout property: 0 turns the depth test off, anything else keeps it.
    mln_plugin_property_value_v1 occlusion{};
    occlusion.struct_size = sizeof(occlusion);
    occlusion.name = {"model-occlusion", 15};
    occlusion.value.struct_size = sizeof(occlusion.value);
    occlusion.value.type = MLN_PLUGIN_VALUE_FLOAT;
    occlusion.value.data.float_value = 0.0f;
    bucket = layOut(layer, {{0, 0}}, {triangle}, instance, 3, &occlusion);
    CHECK(bucket.drawable_count == 1 && bucket.drawables[0].depth_mode == MLN_PLUGIN_DRAWABLE_DEPTH_DISABLED);
    layer.destroy_layout(instance);
    occlusion.value.data.float_value = 1.0f;
    bucket = layOut(layer, {{0, 0}}, {triangle}, instance, 3, &occlusion);
    CHECK(bucket.drawables[0].depth_mode == MLN_PLUGIN_DRAWABLE_DEPTH_READ_WRITE);
    layer.destroy_layout(instance);

    // Enough points to overflow one 16-bit segment: the layout must split rather than wrap.
    std::vector<mln_plugin_tile_point_v1> many(40000);
    for (size_t i = 0; i < many.size(); ++i) many[i] = {static_cast<int16_t>(i % 8000), static_cast<int16_t>(i / 8000)};
    bucket = layOut(layer, many, {triangle}, instance);
    CHECK(bucket.drawables[0].segment_count >= 2);
    for (size_t i = 0; i < bucket.drawables[0].segment_count; ++i) {
        CHECK(bucket.drawables[0].segments[i].vertex_length <= 65535);
    }
    layer.destroy_layout(instance);

    // The host's pixels-per-meter reaches the shader through view.z.
    mln_plugin_uniform_context_v1 uniformContext{};
    uniformContext.struct_size = sizeof(uniformContext);
    uniformContext.pixels_per_meter = 4.5f;
    std::vector<uint8_t> output(uniform.byte_size);
    CHECK(layer.update_uniform_block(&uniformContext, 0, output.data(), output.size()) == MLN_PLUGIN_STATUS_OK);
    CHECK(reinterpret_cast<const float*>(output.data())[20 + 2] == 4.5f);

    // One layer, one tile, many models: every feature draws the model its `model-id` resolved to, and each
    // feature's vertices carry its own model's material.
    constexpr int modelCount = 100;
    std::vector<void*> models;
    for (int i = 0; i < modelCount; ++i)
        models.push_back(createModel(layer, triangleGLB(std::to_string(i / 100.0) + ",0,0,1")));
    std::vector<void*> featureModels;
    for (int i = 0; i < 2 * modelCount; ++i) featureModels.push_back(models[i % modelCount]);
    bucket = layOut(layer, {{5, 5}}, featureModels, instance, 100);
    CHECK(bucket.drawable_count == 1 && bucket.feature_vertex_range_count == featureModels.size());
    CHECK(bucket.index_count == 3 * featureModels.size());
    for (size_t i = 0; i < bucket.feature_vertex_range_count; ++i) {
        const auto& range = bucket.feature_vertex_ranges[i];
        CHECK(range.feature_index == 100 + i && range.vertex_count == 3);
        const float* color = reinterpret_cast<const float*>(bucket.vertex_streams[0].data + range.first_vertex * 44 +
                                                            28);
        CHECK(color[0] == static_cast<float>((i % modelCount) / 100.0));
    }
    layer.destroy_layout(instance);

    for (void* model : models) layer.destroy_resource(model);
    layer.destroy_resource(triangle);

    std::cout << "gltf descriptor, parser and layout tests passed\n";
}
