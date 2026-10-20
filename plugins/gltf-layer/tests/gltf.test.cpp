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
std::vector<uint8_t> triangleGLB() {
    std::string json =
        R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],)"
        R"("meshes":[{"primitives":[{"attributes":{"POSITION":0},"indices":1,"material":0}]}],)"
        R"("materials":[{"pbrMetallicRoughness":{"baseColorFactor":[1,0.5,0.25,1]}}],)"
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

mln_plugin_bucket_v1 layOut(const mln_plugin_layer_type_v1& layer,
                            const std::vector<mln_plugin_tile_point_v1>& points,
                            void*& instance) {
    mln_plugin_layout_context_v1 context{};
    context.struct_size = sizeof(context);
    context.extent = 8192;
    instance = nullptr;
    CHECK(layer.create_layout(&context, &instance) == MLN_PLUGIN_STATUS_OK);
    mln_plugin_feature_v1 feature{};
    feature.struct_size = sizeof(feature);
    feature.geometry_type = MLN_PLUGIN_GEOMETRY_POINT;
    feature.feature_index = 3;
    feature.points = points.data();
    feature.point_count = points.size();
    CHECK(layer.layout_feature(instance, &feature) == MLN_PLUGIN_STATUS_OK);
    mln_plugin_bucket_v1 bucket{};
    bucket.struct_size = sizeof(bucket);
    CHECK(layer.finish_layout(instance, &bucket) == MLN_PLUGIN_STATUS_OK);
    return bucket;
}

// A host that serves one url and fails every other.
std::vector<uint8_t> servedGLB;
int resourceRequests = 0;
mln_plugin_status requestResource(void*,
                                  mln_plugin_string url,
                                  mln_plugin_resource_callback_fn callback,
                                  void* callbackContext,
                                  uint64_t* requestID) {
    ++resourceRequests;
    *requestID = 1;
    mln_plugin_resource_response_v1 response{};
    response.struct_size = sizeof(response);
    static const char missing[] = "not found";
    if (std::string_view(url.data, url.size) == "test://triangle.glb") {
        response.data = servedGLB.data();
        response.data_size = servedGLB.size();
    } else {
        response.error_message = {missing, sizeof(missing) - 1};
    }
    callback(callbackContext, &response);
    return MLN_PLUGIN_STATUS_OK;
}

mln_plugin_status createWithModelURI(const mln_plugin_layer_type_v1& layer, const char* uri, void*& instance) {
    mln_plugin_host_api_v1 host{};
    host.struct_size = sizeof(host);
    host.request_resource = requestResource;
    mln_plugin_property_value_v1 property{};
    property.struct_size = sizeof(property);
    property.name = {"model-uri", 9};
    property.value.struct_size = sizeof(property.value);
    property.value.type = MLN_PLUGIN_VALUE_STRING;
    property.value.data.string_value = {uri, std::strlen(uri)};
    mln_plugin_layout_context_v1 context{};
    context.struct_size = sizeof(context);
    context.extent = 8192;
    context.properties = &property;
    context.property_count = 1;
    context.host = &host;
    instance = nullptr;
    return layer.create_layout(&context, &instance);
}
} // namespace

int main() {
    CHECK(mln_gltf_layer_register(nullptr, nullptr, 0) == MLN_PLUGIN_STATUS_NOT_FOUND);
    CHECK(mln_gltf_layer_register(capture, nullptr, 0) == MLN_PLUGIN_STATUS_OK);
    CHECK(registered && registered->layer_type_count == 1);
    const auto& layer = registered->layer_types[0];
    CHECK(std::string_view(layer.layer_type.data, layer.layer_type.size) == "gltf");
    CHECK(layer.backend_mask == MLN_PLUGIN_BACKEND_METAL && layer.enable_near_clipped_matrix);

    // Every property is bound to the shader, and the binding offsets stay inside the uniform block.
    const auto& shader = layer.shaders[0];
    // `model-uri` is a layout property: read at layout time, not bound.
    CHECK(shader.property_binding_count + 1 == layer.property_count);
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

    // Without a mesh a tile lays out to nothing instead of failing.
    void* instance = nullptr;
    auto bucket = layOut(layer, {{10, 10}}, instance);
    CHECK(bucket.drawable_count == 0 && bucket.index_count == 0);
    layer.destroy_layout(instance);

    // A malformed asset is rejected with a diagnostic and leaves the mesh unset.
    char message[256] = {};
    const uint8_t garbage[] = {1, 2, 3, 4, 5, 6, 7, 8};
    CHECK(mln_gltf_layer_set_model(garbage, sizeof(garbage), message, sizeof(message)) ==
          MLN_PLUGIN_STATUS_INVALID_ARGUMENT);
    CHECK(message[0] != '\0');

    const auto glb = triangleGLB();
    CHECK(mln_gltf_layer_set_model(glb.data(), glb.size(), message, sizeof(message)) == MLN_PLUGIN_STATUS_OK);
    CHECK(message[0] == '\0');

    // Two in-tile points; the out-of-tile pair belongs to the neighbouring tiles.
    bucket = layOut(layer, {{0, 0}, {8191, 8191}, {8192, 0}, {-1, 0}}, instance);
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

    // Enough points to overflow one 16-bit segment: the layout must split rather than wrap.
    std::vector<mln_plugin_tile_point_v1> many(40000);
    for (size_t i = 0; i < many.size(); ++i) many[i] = {static_cast<int16_t>(i % 8000), static_cast<int16_t>(i / 8000)};
    bucket = layOut(layer, many, instance);
    CHECK(bucket.drawables[0].segment_count >= 2);
    for (size_t i = 0; i < bucket.drawables[0].segment_count; ++i) {
        CHECK(bucket.drawables[0].segments[i].vertex_length <= 65535);
    }
    layer.destroy_layout(instance);

    // The host's pixels-per-meter reaches the shader through view.z; an older host leaves it at 0.
    mln_plugin_uniform_context_v1 uniformContext{};
    uniformContext.struct_size = sizeof(uniformContext);
    uniformContext.pixels_per_meter = 4.5f;
    std::vector<uint8_t> output(uniform.byte_size);
    CHECK(layer.update_uniform_block(&uniformContext, 0, output.data(), output.size()) == MLN_PLUGIN_STATUS_OK);
    CHECK(reinterpret_cast<const float*>(output.data())[20 + 2] == 4.5f);
    uniformContext.struct_size = offsetof(mln_plugin_uniform_context_v1, pixels_per_meter);
    CHECK(layer.update_uniform_block(&uniformContext, 0, output.data(), output.size()) == MLN_PLUGIN_STATUS_OK);
    CHECK(reinterpret_cast<const float*>(output.data())[20 + 2] == 0.0f);

    // A layer's `model-uri` is loaded through the host and cached per uri.
    CHECK(mln_gltf_layer_set_model(nullptr, 0, nullptr, 0) == MLN_PLUGIN_STATUS_OK);
    servedGLB = glb;
    CHECK(createWithModelURI(layer, "test://triangle.glb", instance) == MLN_PLUGIN_STATUS_OK);
    mln_plugin_feature_v1 uriFeature{};
    const mln_plugin_tile_point_v1 uriPoint{5, 5};
    uriFeature.struct_size = sizeof(uriFeature);
    uriFeature.geometry_type = MLN_PLUGIN_GEOMETRY_POINT;
    uriFeature.points = &uriPoint;
    uriFeature.point_count = 1;
    CHECK(layer.layout_feature(instance, &uriFeature) == MLN_PLUGIN_STATUS_OK);
    bucket = {};
    bucket.struct_size = sizeof(bucket);
    CHECK(layer.finish_layout(instance, &bucket) == MLN_PLUGIN_STATUS_OK);
    CHECK(bucket.drawable_count == 1 && bucket.index_count == 3);
    layer.destroy_layout(instance);
    CHECK(resourceRequests == 1);
    CHECK(createWithModelURI(layer, "test://triangle.glb", instance) == MLN_PLUGIN_STATUS_OK);
    layer.destroy_layout(instance);
    CHECK(resourceRequests == 1);

    // A feature can name its own model through a `model-uri` attribute; it overrides the layer's, and a layer
    // with no model of its own still draws the features that name one. A feature naming a model that cannot be
    // loaded is skipped without failing the tile.
    CHECK(createWithModelURI(layer, "", instance) == MLN_PLUGIN_STATUS_OK);
    const auto layOutFeature = [&](const char* json, uint64_t index) {
        mln_plugin_feature_v1 feature{};
        feature.struct_size = sizeof(feature);
        feature.geometry_type = MLN_PLUGIN_GEOMETRY_POINT;
        feature.feature_index = index;
        feature.points = &uriPoint;
        feature.point_count = 1;
        feature.properties_json = {json, std::strlen(json)};
        return layer.layout_feature(instance, &feature);
    };
    CHECK(layOutFeature(R"({"name":"a"})", 1) == MLN_PLUGIN_STATUS_OK);                           // no model: skipped
    CHECK(layOutFeature(R"({"model-uri":"test://missing.glb"})", 2) == MLN_PLUGIN_STATUS_OK);       // unloadable: skipped
    CHECK(layOutFeature(R"({"model-uri":"test://triangle.glb"})", 3) == MLN_PLUGIN_STATUS_OK);
    CHECK(layOutFeature(R"({"model-uri":7})", 4) == MLN_PLUGIN_STATUS_OK);                          // not a string: skipped
    CHECK(layOutFeature("not json", 5) == MLN_PLUGIN_STATUS_OK);
    bucket = {};
    bucket.struct_size = sizeof(bucket);
    CHECK(layer.finish_layout(instance, &bucket) == MLN_PLUGIN_STATUS_OK);
    CHECK(bucket.feature_vertex_range_count == 1 && bucket.feature_vertex_ranges[0].feature_index == 3);
    CHECK(bucket.index_count == 3);
    layer.destroy_layout(instance);

    // A uri that cannot be loaded fails the layout instead of drawing another model.
    CHECK(createWithModelURI(layer, "test://missing.glb", instance) == MLN_PLUGIN_STATUS_CALLBACK_ERROR);
    if (instance) layer.destroy_layout(instance);

    // Clearing the mesh makes later tiles lay out to nothing again.
    CHECK(mln_gltf_layer_set_model(glb.data(), glb.size(), nullptr, 0) == MLN_PLUGIN_STATUS_OK);
    CHECK(mln_gltf_layer_set_model(nullptr, 0, nullptr, 0) == MLN_PLUGIN_STATUS_OK);
    bucket = layOut(layer, {{1, 1}}, instance);
    CHECK(bucket.drawable_count == 0);
    layer.destroy_layout(instance);

    std::cout << "gltf descriptor, parser and layout tests passed\n";
}
