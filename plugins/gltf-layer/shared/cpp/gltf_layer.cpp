#include "gltf_layer.hpp"

#include <tiny_gltf.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gltf_shader_metal.hpp"

#ifndef MLN_GLTF_PLUGIN_VERSION
#define MLN_GLTF_PLUGIN_VERSION "0.1.0-local"
#endif

namespace {

constexpr uint32_t vertexStream = 0;
constexpr uint64_t modelDrawable = 1;
// Guards against a cyclic or absurdly deep node graph in an untrusted asset.
constexpr int maxNodeDepth = 64;

constexpr mln_plugin_string str(const char* value, size_t size) {
    return {value, size};
}

template <size_t N>
constexpr mln_plugin_string str(const char (&value)[N]) {
    return str(value, N - 1);
}

struct Matrix {
    double value[16]{};
};
struct ModelVertex {
    float position[3];
    float normal[3];
    float color[4];
};
struct Model {
    std::vector<ModelVertex> vertices;
    std::vector<uint32_t> indices;
};

// `position` is only the anchor in tile units. The model-space `model` offset is combined with it in the
// vertex shader, so heading, size and opacity are evaluated every frame instead of when the tile is laid
// out.
struct Vertex {
    int16_t position[2];
    float model[3];
    float normal[3];
    float color[4];
};
static_assert(offsetof(Vertex, model) == 4 && offsetof(Vertex, normal) == 16 && offsetof(Vertex, color) == 28);
static_assert(sizeof(Vertex) == 44);

struct Layout {
    const mln_plugin_host_api_v1* host = nullptr;
    // The layer's model: its `model-uri`, or else the mesh set through mln_gltf_layer_set_model. Null when
    // neither is given; a layer whose features all name their own model needs none.
    std::shared_ptr<const Model> model;
    uint32_t extent = 8192;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<mln_plugin_segment_v1> segments;
    std::vector<mln_plugin_feature_vertex_range_v1> featureRanges;
    std::array<mln_plugin_vertex_stream_v1, 1> streams{};
    std::array<mln_plugin_attribute_binding_v1, 4> attributes{};
    std::array<mln_plugin_drawable_descriptor_v1, 1> drawables{};
};

std::mutex currentModelMutex;
std::shared_ptr<const Model> currentModel;

Matrix identity() {
    Matrix result{};
    result.value[0] = result.value[5] = result.value[10] = result.value[15] = 1.0;
    return result;
}

Matrix multiply(const Matrix& a, const Matrix& b) {
    Matrix result{};
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            for (int k = 0; k < 4; ++k) {
                result.value[column * 4 + row] += a.value[k * 4 + row] * b.value[column * 4 + k];
            }
        }
    }
    return result;
}

Matrix nodeMatrix(const tinygltf::Node& node) {
    if (node.matrix.size() == 16) {
        Matrix result{};
        std::copy(node.matrix.begin(), node.matrix.end(), result.value);
        return result;
    }
    Matrix translation = identity();
    if (node.translation.size() == 3) {
        translation.value[12] = node.translation[0];
        translation.value[13] = node.translation[1];
        translation.value[14] = node.translation[2];
    }
    Matrix rotation = identity();
    if (node.rotation.size() == 4) {
        const double x = node.rotation[0], y = node.rotation[1], z = node.rotation[2], w = node.rotation[3];
        rotation.value[0] = 1.0 - 2.0 * (y * y + z * z);
        rotation.value[1] = 2.0 * (x * y + z * w);
        rotation.value[2] = 2.0 * (x * z - y * w);
        rotation.value[4] = 2.0 * (x * y - z * w);
        rotation.value[5] = 1.0 - 2.0 * (x * x + z * z);
        rotation.value[6] = 2.0 * (y * z + x * w);
        rotation.value[8] = 2.0 * (x * z + y * w);
        rotation.value[9] = 2.0 * (y * z - x * w);
        rotation.value[10] = 1.0 - 2.0 * (x * x + y * y);
    }
    Matrix scale = identity();
    if (node.scale.size() == 3) {
        scale.value[0] = node.scale[0];
        scale.value[5] = node.scale[1];
        scale.value[10] = node.scale[2];
    }
    return multiply(translation, multiply(rotation, scale));
}

double component(const tinygltf::Model& model,
                 const tinygltf::Accessor& accessor,
                 size_t element,
                 size_t componentIndex) {
    if (accessor.bufferView < 0 || static_cast<size_t>(accessor.bufferView) >= model.bufferViews.size()) return 0.0;
    const auto& view = model.bufferViews[accessor.bufferView];
    if (view.buffer < 0 || static_cast<size_t>(view.buffer) >= model.buffers.size()) return 0.0;
    const auto& buffer = model.buffers[view.buffer];
    const size_t componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
    const int byteStride = accessor.ByteStride(view);
    const size_t stride = byteStride > 0
                              ? static_cast<size_t>(byteStride)
                              : componentSize * tinygltf::GetNumComponentsInType(accessor.type);
    const size_t offset = view.byteOffset + accessor.byteOffset + element * stride + componentIndex * componentSize;
    if (offset + componentSize > buffer.data.size()) return 0.0;
    const uint8_t* data = buffer.data.data() + offset;
    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_FLOAT: {
            float value;
            std::memcpy(&value, data, sizeof(value));
            return value;
        }
        case TINYGLTF_COMPONENT_TYPE_DOUBLE: {
            double value;
            std::memcpy(&value, data, sizeof(value));
            return value;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return *data;
        case TINYGLTF_COMPONENT_TYPE_BYTE:
            return *reinterpret_cast<const int8_t*>(data);
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            uint16_t value;
            std::memcpy(&value, data, sizeof(value));
            return value;
        }
        case TINYGLTF_COMPONENT_TYPE_SHORT: {
            int16_t value;
            std::memcpy(&value, data, sizeof(value));
            return value;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            uint32_t value;
            std::memcpy(&value, data, sizeof(value));
            return value;
        }
        default:
            return 0.0;
    }
}

void transformPoint(const Matrix& matrix, const double input[3], float output[3]) {
    for (int row = 0; row < 3; ++row) {
        output[row] = static_cast<float>(matrix.value[row] * input[0] + matrix.value[4 + row] * input[1] +
                                         matrix.value[8 + row] * input[2] + matrix.value[12 + row]);
    }
}

void transformNormal(const Matrix& matrix, const double input[3], float output[3]) {
    double length = 0.0;
    for (int row = 0; row < 3; ++row) {
        const double value = matrix.value[row] * input[0] + matrix.value[4 + row] * input[1] +
                             matrix.value[8 + row] * input[2];
        output[row] = static_cast<float>(value);
        length += value * value;
    }
    length = std::sqrt(length);
    if (length > std::numeric_limits<double>::epsilon()) {
        for (size_t i = 0; i < 3; ++i) output[i] = static_cast<float>(output[i] / length);
    }
}

void appendPrimitive(const tinygltf::Model& source,
                     const tinygltf::Primitive& primitive,
                     const Matrix& transform,
                     Model& output) {
    if (primitive.mode != TINYGLTF_MODE_TRIANGLES || primitive.indices < 0 ||
        static_cast<size_t>(primitive.indices) >= source.accessors.size()) {
        return;
    }
    const auto positionIt = primitive.attributes.find("POSITION");
    if (positionIt == primitive.attributes.end() || positionIt->second < 0 ||
        static_cast<size_t>(positionIt->second) >= source.accessors.size()) {
        return;
    }
    const auto& positions = source.accessors[positionIt->second];
    const auto normalIt = primitive.attributes.find("NORMAL");
    const tinygltf::Accessor* normals = nullptr;
    if (normalIt != primitive.attributes.end() && normalIt->second >= 0 &&
        static_cast<size_t>(normalIt->second) < source.accessors.size()) {
        normals = &source.accessors[normalIt->second];
    }
    // Textures are not part of the plugin ABI, so only the material's base color factor is honored.
    double color[4]{0.72, 0.64, 0.48, 1.0};
    if (primitive.material >= 0 && static_cast<size_t>(primitive.material) < source.materials.size()) {
        const auto& factor = source.materials[primitive.material].pbrMetallicRoughness.baseColorFactor;
        if (factor.size() == 4) std::copy(factor.begin(), factor.end(), color);
    }
    const auto vertexOffset = static_cast<uint32_t>(output.vertices.size());
    for (size_t i = 0; i < positions.count; ++i) {
        const double position[3]{
            component(source, positions, i, 0), component(source, positions, i, 1), component(source, positions, i, 2)};
        const double normal[3]{normals ? component(source, *normals, i, 0) : 0.0,
                               normals ? component(source, *normals, i, 1) : 1.0,
                               normals ? component(source, *normals, i, 2) : 0.0};
        ModelVertex vertex{};
        transformPoint(transform, position, vertex.position);
        transformNormal(transform, normal, vertex.normal);
        for (size_t c = 0; c < 4; ++c) vertex.color[c] = static_cast<float>(color[c]);
        output.vertices.push_back(vertex);
    }
    const auto& indices = source.accessors[primitive.indices];
    for (size_t i = 0; i < indices.count; ++i) {
        const auto index = static_cast<uint32_t>(component(source, indices, i, 0));
        if (index < positions.count) output.indices.push_back(vertexOffset + index);
    }
}

void appendNode(const tinygltf::Model& source, int nodeIndex, const Matrix& parent, int depth, Model& output) {
    if (depth > maxNodeDepth || nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= source.nodes.size()) return;
    const auto& node = source.nodes[nodeIndex];
    const auto transform = multiply(parent, nodeMatrix(node));
    if (node.mesh >= 0 && static_cast<size_t>(node.mesh) < source.meshes.size()) {
        for (const auto& primitive : source.meshes[node.mesh].primitives) {
            appendPrimitive(source, primitive, transform, output);
        }
    }
    for (const int child : node.children) appendNode(source, child, transform, depth + 1, output);
}

std::shared_ptr<const Model> parseGLB(const uint8_t* bytes, size_t size, std::string& error) {
    tinygltf::TinyGLTF loader;
    // Images are never read, so skip decoding them.
    loader.SetImageLoader(
        [](tinygltf::Image*, int, std::string*, std::string*, int, int, const unsigned char*, int, void*) {
            return true;
        },
        nullptr);
    tinygltf::Model source;
    std::string warning;
    if (!loader.LoadBinaryFromMemory(&source, &error, &warning, bytes, static_cast<unsigned int>(size))) {
        if (error.empty()) error = warning.empty() ? "TinyGLTF could not parse the GLB" : warning;
        return {};
    }
    auto result = std::make_shared<Model>();
    const int sceneIndex = source.defaultScene >= 0 ? source.defaultScene : (source.scenes.empty() ? -1 : 0);
    if (sceneIndex >= 0 && static_cast<size_t>(sceneIndex) < source.scenes.size()) {
        for (const int node : source.scenes[sceneIndex].nodes) appendNode(source, node, identity(), 0, *result);
    } else {
        for (size_t node = 0; node < source.nodes.size(); ++node) {
            appendNode(source, static_cast<int>(node), identity(), 0, *result);
        }
    }
    if (result->vertices.empty() || result->indices.size() < 3) {
        error = "GLB contains no indexed triangle meshes";
        return {};
    }
    result->indices.resize(result->indices.size() - result->indices.size() % 3);
    return result;
}

void copyDiagnostic(const std::string& message, char* output, size_t capacity) {
    if (!output || capacity == 0) return;
    const size_t length = std::min(message.size(), capacity - 1);
    std::memcpy(output, message.data(), length);
    output[length] = '\0';
}

void log(const mln_plugin_host_api_v1* host, int32_t severity, const std::string& message) {
    if (host && host->struct_size >= offsetof(mln_plugin_host_api_v1, context) && host->log) {
        host->log(severity, {message.data(), message.size()});
    }
}

struct ModelResponse {
    std::shared_ptr<const Model> model;
    std::string error;
};

void modelLoaded(void* context, const mln_plugin_resource_response_v1* response) {
    auto& result = *static_cast<ModelResponse*>(context);
    if (!response || !response->data || response->data_size == 0) {
        result.error = response && response->error_message.data
                           ? std::string(response->error_message.data, response->error_message.size)
                           : "empty response";
        return;
    }
    result.model = parseGLB(response->data, response->data_size, result.error);
}

// Tiles share one parsed model per uri. Failures are not cached, so a later tile retries.
std::mutex modelCacheMutex;
std::unordered_map<std::string, std::shared_ptr<const Model>> modelCache;

std::shared_ptr<const Model> loadModel(const mln_plugin_host_api_v1* host, const std::string& uri) {
    {
        std::lock_guard lock(modelCacheMutex);
        const auto found = modelCache.find(uri);
        if (found != modelCache.end()) return found->second;
    }
    if (!host || host->struct_size < sizeof(*host) || !host->request_resource) {
        log(host, 3, "gltf: the host cannot load resources");
        return {};
    }
    ModelResponse response;
    uint64_t requestID = 0;
    const auto status = host->request_resource(host->context, {uri.data(), uri.size()}, modelLoaded, &response, &requestID);
    if (status != MLN_PLUGIN_STATUS_OK || !response.model) {
        log(host, 3, "gltf: failed to load '" + uri + "': " + (response.error.empty() ? "request rejected" : response.error));
        return {};
    }
    std::lock_guard lock(modelCacheMutex);
    return modelCache.emplace(uri, std::move(response.model)).first->second;
}

// A string layout property by name; empty when absent or when the host predates layout properties.
std::string layoutString(const mln_plugin_layout_context_v1& context, std::string_view name) {
    constexpr size_t required = offsetof(mln_plugin_layout_context_v1, host) + sizeof(void*);
    if (context.struct_size < required) return {};
    for (size_t i = 0; i < context.property_count; ++i) {
        const auto& property = context.properties[i];
        if (std::string_view(property.name.data, property.name.size) != name) continue;
        if (property.value.type != MLN_PLUGIN_VALUE_STRING || !property.value.data.string_value.data) return {};
        return std::string(property.value.data.string_value.data, property.value.data.string_value.size);
    }
    return {};
}

void startSegment(Layout& layout) {
    mln_plugin_segment_v1 segment{};
    segment.struct_size = sizeof(segment);
    segment.vertex_offset = static_cast<uint32_t>(layout.vertices.size());
    segment.index_offset = static_cast<uint32_t>(layout.indices.size());
    layout.segments.push_back(segment);
}

mln_plugin_status createLayout(const mln_plugin_layout_context_v1* context, void** instance) try {
    if (!context || context->struct_size < sizeof(*context) || !instance) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto layout = std::unique_ptr<Layout>(new (std::nothrow) Layout());
    if (!layout) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    layout->host = context->struct_size >= offsetof(mln_plugin_layout_context_v1, host) + sizeof(void*) ? context->host : nullptr;
    // The layer's `model-uri` wins; a layer without one draws the mesh set through mln_gltf_layer_set_model.
    // A uri that is given but cannot be loaded fails the layout rather than silently drawing another model.
    const auto uri = layoutString(*context, "model-uri");
    if (!uri.empty()) {
        layout->model = loadModel(layout->host, uri);
        if (!layout->model) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
    } else {
        std::lock_guard lock(currentModelMutex);
        layout->model = currentModel;
    }
    layout->extent = context->extent;
    startSegment(*layout);
    *instance = layout.release();
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}

mln_plugin_status layoutFeature(void* instance, const mln_plugin_feature_v1* feature) try {
    if (!instance || !feature || feature->struct_size < sizeof(*feature) ||
        feature->geometry_type != MLN_PLUGIN_GEOMETRY_POINT || !feature->points) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto& layout = *static_cast<Layout*>(instance);
    // No mesh set yet: draw nothing rather than fail the tile.
    if (!layout.model) return MLN_PLUGIN_STATUS_OK;
    const auto& model = *layout.model;
    const auto firstVertex = static_cast<uint32_t>(layout.vertices.size());

    for (size_t pointIndex = 0; pointIndex < feature->point_count; ++pointIndex) {
        const auto anchor = feature->points[pointIndex];
        // GeoJSON tile buffers can repeat the same point in adjacent tiles. Emit it only from the
        // canonical tile that owns the anchor.
        if (anchor.x < 0 || anchor.y < 0 || anchor.x >= static_cast<int32_t>(layout.extent) ||
            anchor.y >= static_cast<int32_t>(layout.extent)) {
            continue;
        }
        // Indices are 16-bit and relative to the segment, so a triangle that would overflow one starts a new
        // segment and the mesh vertices are re-emitted into it.
        std::unordered_map<uint32_t, uint16_t> localIndices;
        for (size_t triangle = 0; triangle + 2 < model.indices.size(); triangle += 3) {
            size_t missing = 0;
            for (size_t corner = 0; corner < 3; ++corner) {
                missing += localIndices.count(model.indices[triangle + corner]) == 0;
            }
            if (layout.segments.back().vertex_length + missing > std::numeric_limits<uint16_t>::max()) {
                startSegment(layout);
                localIndices.clear();
            }
            auto& segment = layout.segments.back();
            for (size_t corner = 0; corner < 3; ++corner) {
                const uint32_t modelIndex = model.indices[triangle + corner];
                if (modelIndex >= model.vertices.size()) return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
                auto found = localIndices.find(modelIndex);
                if (found == localIndices.end()) {
                    const auto localIndex = static_cast<uint16_t>(segment.vertex_length);
                    const auto& source = model.vertices[modelIndex];
                    Vertex vertex{};
                    vertex.position[0] = anchor.x;
                    vertex.position[1] = anchor.y;
                    std::copy_n(source.position, 3, vertex.model);
                    std::copy_n(source.normal, 3, vertex.normal);
                    std::copy_n(source.color, 4, vertex.color);
                    layout.vertices.push_back(vertex);
                    found = localIndices.emplace(modelIndex, localIndex).first;
                    ++segment.vertex_length;
                }
                layout.indices.push_back(found->second);
                ++segment.index_length;
            }
        }
    }

    if (layout.vertices.size() > firstVertex) {
        layout.featureRanges.push_back({sizeof(mln_plugin_feature_vertex_range_v1),
                                        feature->feature_index,
                                        modelDrawable,
                                        firstVertex,
                                        static_cast<uint32_t>(layout.vertices.size() - firstVertex)});
    }
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}

mln_plugin_status finishLayout(void* instance, mln_plugin_bucket_v1* output) {
    if (!instance || !output || output->struct_size < sizeof(*output)) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    auto& layout = *static_cast<Layout*>(instance);
    layout.segments.erase(std::remove_if(layout.segments.begin(),
                                         layout.segments.end(),
                                         [](const auto& segment) { return segment.index_length == 0; }),
                          layout.segments.end());

    if (!layout.vertices.empty()) {
        layout.streams[0] = {sizeof(mln_plugin_vertex_stream_v1),
                             vertexStream,
                             reinterpret_cast<const uint8_t*>(layout.vertices.data()),
                             layout.vertices.size() * sizeof(Vertex),
                             static_cast<uint32_t>(layout.vertices.size()),
                             sizeof(Vertex)};
    }

    // Attributes 1-5 are the paint properties, which the host binds itself.
    layout.attributes = {{
        {sizeof(mln_plugin_attribute_binding_v1), 0, vertexStream, offsetof(Vertex, position)},
        {sizeof(mln_plugin_attribute_binding_v1), 6, vertexStream, offsetof(Vertex, model)},
        {sizeof(mln_plugin_attribute_binding_v1), 7, vertexStream, offsetof(Vertex, normal)},
        {sizeof(mln_plugin_attribute_binding_v1), 8, vertexStream, offsetof(Vertex, color)},
    }};

    if (!layout.segments.empty()) {
        auto& drawable = layout.drawables[0];
        drawable = {};
        drawable.struct_size = sizeof(drawable);
        drawable.drawable_key = modelDrawable;
        drawable.shader_id = str("gltf");
        drawable.attributes = layout.attributes.data();
        drawable.attribute_count = layout.attributes.size();
        drawable.segments = layout.segments.data();
        drawable.segment_count = layout.segments.size();
        // Real depth, so the mesh orders against itself and against the map's extrusions. glTF materials
        // may be double sided, and the plugin does not read that flag, so nothing is culled.
        drawable.depth_mode = MLN_PLUGIN_DRAWABLE_DEPTH_READ_WRITE;
        drawable.enable_stencil_overlap = 0;
        drawable.cull_back_faces = 0;
    }

    output->vertex_streams = layout.vertices.empty() ? nullptr : layout.streams.data();
    output->vertex_stream_count = layout.vertices.empty() ? 0 : 1;
    output->indices = layout.indices.data();
    output->index_count = layout.indices.size();
    output->drawables = layout.segments.empty() ? nullptr : layout.drawables.data();
    output->drawable_count = layout.segments.empty() ? 0 : 1;
    output->query_radius = 0.0f;
    output->feature_vertex_ranges = layout.featureRanges.data();
    output->feature_vertex_range_count = layout.featureRanges.size();
    return MLN_PLUGIN_STATUS_OK;
}

void destroyLayout(void* instance) {
    delete static_cast<Layout*>(instance);
}

// Must stay layout-compatible with GltfUBO in gltf_shader_metal.hpp. Everything from `heading` onwards is
// written by the host through the property bindings below, never by this plugin.
struct alignas(16) DrawableUBO {
    float matrix[16];
    float camera[4];
    float view[4];
    float heading, scale, screenScale, opacity;
    float altitude[4];
    float interpolation[16];
};
static_assert(offsetof(DrawableUBO, heading) == 96);
static_assert(offsetof(DrawableUBO, altitude) == 112);
static_assert(offsetof(DrawableUBO, interpolation) == 128);
static_assert(sizeof(DrawableUBO) == 192);

mln_plugin_status updateUniform(const mln_plugin_uniform_context_v1* context,
                                uint32_t id,
                                uint8_t* output,
                                size_t size) {
    if (!context || !output || id != 0 || size != sizeof(DrawableUBO)) {
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    DrawableUBO value{};
    std::copy_n(context->tile_matrix, 16, value.matrix);
    value.camera[0] = context->pixels_to_gl_units[0];
    value.camera[1] = context->pixels_to_gl_units[1];
    value.camera[2] = context->pixels_to_tile_units;
    value.camera[3] = context->camera_to_center_distance;
    value.view[0] = static_cast<float>(context->bearing);
    value.view[1] = context->pixel_ratio;
    // An older host without the field leaves it at 0, which the shader treats as 1.
    if (context->struct_size >= offsetof(mln_plugin_uniform_context_v1, pixels_per_meter) + sizeof(float)) {
        value.view[2] = context->pixels_per_meter;
    }
    std::memcpy(output, &value, sizeof(value));
    return MLN_PLUGIN_STATUS_OK;
}

constexpr mln_plugin_value number(float n) {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_FLOAT;
    v.data.float_value = n;
    return v;
}

constexpr mln_plugin_value text(mln_plugin_string t) {
    mln_plugin_value v{};
    v.struct_size = sizeof(v);
    v.type = MLN_PLUGIN_VALUE_STRING;
    v.data.string_value = t;
    return v;
}

// A layout property is read once per tile by create_layout; it takes no expressions or transitions.
constexpr mln_plugin_property_descriptor_v1 layoutProperty(mln_plugin_string name, mln_plugin_value value) {
    mln_plugin_property_descriptor_v1 p{};
    p.struct_size = sizeof(p);
    p.name = name;
    p.type = value.type;
    p.default_value = value;
    p.scope = MLN_PLUGIN_PROPERTY_LAYOUT;
    return p;
}

constexpr mln_plugin_property_descriptor_v1 property(mln_plugin_string name,
                                                     mln_plugin_value value,
                                                     float minimum = -std::numeric_limits<float>::infinity(),
                                                     float maximum = std::numeric_limits<float>::infinity()) {
    mln_plugin_property_descriptor_v1 p{};
    p.struct_size = sizeof(p);
    p.name = name;
    p.type = value.type;
    p.default_value = value;
    p.expression_capabilities = MLN_PLUGIN_EXPRESSION_CAMERA | MLN_PLUGIN_EXPRESSION_FEATURE |
                                MLN_PLUGIN_EXPRESSION_COMPOSITE | MLN_PLUGIN_EXPRESSION_FEATURE_STATE;
    p.supports_transitions = true;
    p.has_minimum = minimum != -std::numeric_limits<float>::infinity();
    p.minimum = p.has_minimum ? minimum : 0;
    p.has_maximum = maximum != std::numeric_limits<float>::infinity();
    p.maximum = p.has_maximum ? maximum : 0;
    return p;
}

const mln_plugin_property_descriptor_v1 properties[] = {
    // A .glb with embedded buffers, loaded through the host's file source: an asset://, file:// or https:// url.
    // Optional: without it the layer draws the mesh set through mln_gltf_layer_set_model.
    layoutProperty(str("model-uri"), text(str(""))),
    // Degrees clockwise from north, applied on top of the mesh's own forward direction.
    property(str("model-heading"), number(0)),
    // Meters per mesh unit.
    property(str("model-scale"), number(1), 0),
    // Points per mesh unit. When above zero the mesh keeps that size on screen at every zoom and
    // `model-scale` is ignored.
    property(str("model-screen-scale"), number(0), 0),
    property(str("model-opacity"), number(1), 0, 1),
    // Meters above the ground.
    property(str("model-altitude"), number(0)),
};

const mln_plugin_shader_attribute_v1 shaderAttributes[] = {
    {sizeof(mln_plugin_shader_attribute_v1), 0, 0, str("a_position"), MLN_PLUGIN_VERTEX_INT16_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 1, 1, str("a_heading"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 2, 2, str("a_scale"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 3, 3, str("a_screen_scale"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 4, 4, str("a_opacity"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 5, 5, str("a_altitude"), MLN_PLUGIN_VERTEX_FLOAT_X2},
    {sizeof(mln_plugin_shader_attribute_v1), 6, 6, str("a_model"), MLN_PLUGIN_VERTEX_FLOAT_X3},
    {sizeof(mln_plugin_shader_attribute_v1), 7, 7, str("a_normal"), MLN_PLUGIN_VERTEX_FLOAT_X3},
    {sizeof(mln_plugin_shader_attribute_v1), 8, 8, str("a_color"), MLN_PLUGIN_VERTEX_FLOAT_X4},
};
static_assert(std::size(shaderAttributes) <= MLN_PLUGIN_MAX_VERTEX_ATTRIBUTES);

constexpr mln_plugin_shader_property_binding_v1 binding(mln_plugin_string name,
                                                        uint32_t uniformOffset,
                                                        uint32_t attribute,
                                                        uint32_t interpolationIndex) {
    mln_plugin_shader_property_binding_v1 b{};
    b.struct_size = sizeof(b);
    b.property_name = name;
    b.encoding = MLN_PLUGIN_PROPERTY_ENCODING_FLOAT;
    b.uniform_id = 0;
    b.uniform_byte_offset = uniformOffset;
    b.minimum_attribute_id = attribute;
    b.maximum_attribute_id = attribute;
    b.interpolation_uniform_id = 0;
    b.interpolation_uniform_byte_offset =
        static_cast<uint32_t>(offsetof(DrawableUBO, interpolation) + interpolationIndex * sizeof(float));
    return b;
}

// The interpolation index doubles as the float slot the shader reads as u.interpolation[i / 4][i % 4].
const mln_plugin_shader_property_binding_v1 propertyBindings[] = {
    binding(str("model-heading"), offsetof(DrawableUBO, heading), 1, 0),
    binding(str("model-scale"), offsetof(DrawableUBO, scale), 2, 1),
    binding(str("model-screen-scale"), offsetof(DrawableUBO, screenScale), 3, 2),
    binding(str("model-opacity"), offsetof(DrawableUBO, opacity), 4, 3),
    binding(str("model-altitude"), offsetof(DrawableUBO, altitude), 5, 4),
};

const mln_plugin_shader_source_v1 shaderSources[] = {
    {sizeof(mln_plugin_shader_source_v1),
     MLN_PLUGIN_BACKEND_METAL,
     str(gltfMetalSource),
     {},
     str("gltfVertex"),
     str("gltfFragment")},
};
const mln_plugin_uniform_block_descriptor_v1 uniforms[] = {
    {sizeof(mln_plugin_uniform_block_descriptor_v1),
     0,
     str("GltfUBO"),
     sizeof(DrawableUBO),
     MLN_PLUGIN_SHADER_STAGE_VERTEX,
     MLN_PLUGIN_UNIFORM_DRAWABLE},
};
const mln_plugin_shader_descriptor_v1 shaders[] = {
    {sizeof(mln_plugin_shader_descriptor_v1),
     str("gltf"),
     shaderSources,
     std::size(shaderSources),
     shaderAttributes,
     std::size(shaderAttributes),
     uniforms,
     std::size(uniforms),
     propertyBindings,
     std::size(propertyBindings)},
};
const mln_plugin_layer_type_v1 layerType = [] {
    mln_plugin_layer_type_v1 v{};
    v.struct_size = sizeof(v);
    v.layer_type = str("gltf");
    v.backend_mask = MLN_PLUGIN_BACKEND_METAL;
    v.properties = properties;
    v.property_count = std::size(properties);
    v.geometry_type_mask = MLN_PLUGIN_GEOMETRY_POINT;
    v.shaders = shaders;
    v.shader_count = std::size(shaders);
    v.create_layout = createLayout;
    v.layout_feature = layoutFeature;
    v.finish_layout = finishLayout;
    v.destroy_layout = destroyLayout;
    v.update_uniform_block = updateUniform;
    v.enable_stencil_overlap_dedup = 0;
    v.enable_near_clipped_matrix = 1;
    return v;
}();
const mln_plugin_descriptor_v1 descriptor = {
    sizeof(mln_plugin_descriptor_v1),
    MLN_PLUGIN_ABI_VERSION_1,
    str("org.maplibre.gltf-layer"),
    str(MLN_GLTF_PLUGIN_VERSION),
    MLN_PLUGIN_ABI_VERSION_1,
    MLN_PLUGIN_ABI_VERSION_1,
    &layerType,
    1,
};

} // namespace

extern "C" mln_plugin_status mln_gltf_layer_register(mln_plugin_register_function_v1 registerPlugin,
                                                      char* error,
                                                      size_t capacity) {
    if (!registerPlugin) return MLN_PLUGIN_STATUS_NOT_FOUND;
    return registerPlugin(&descriptor, error, capacity);
}

extern "C" mln_plugin_status mln_gltf_layer_set_model(const uint8_t* bytes,
                                                       size_t size,
                                                       char* error,
                                                       size_t capacity) try {
    copyDiagnostic({}, error, capacity);
    if (!bytes || size == 0) {
        std::lock_guard lock(currentModelMutex);
        currentModel.reset();
        return MLN_PLUGIN_STATUS_OK;
    }
    std::string message;
    auto model = parseGLB(bytes, size, message);
    if (!model) {
        copyDiagnostic(message, error, capacity);
        return MLN_PLUGIN_STATUS_INVALID_ARGUMENT;
    }
    std::lock_guard lock(currentModelMutex);
    currentModel = std::move(model);
    return MLN_PLUGIN_STATUS_OK;
} catch (...) {
    copyDiagnostic("unexpected failure while reading the model", error, capacity);
    return MLN_PLUGIN_STATUS_CALLBACK_ERROR;
}
