#include "gltf_loader.hpp"

#include <mbgl/util/logging.hpp>
#include <mbgl/util/premultiply.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include <cassert>

namespace mbgl {
namespace util {

namespace {

size_t getAccessorCount(const tinygltf::Model& model, int accessorIdx) {
    return model.accessors[accessorIdx].count;
}

int getAccessorStride(const tinygltf::Model& model, int accessorIdx) {
    const auto& accessor = model.accessors[accessorIdx];
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    if (bufferView.byteStride > 0) {
        return static_cast<int>(bufferView.byteStride);
    }
    // Default stride from component size * type count
    int componentSize = 1;
    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            componentSize = 1;
            break;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            componentSize = 2;
            break;
        case TINYGLTF_COMPONENT_TYPE_INT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            componentSize = 4;
            break;
        default:
            break;
    }
    int typeCount = 1;
    switch (accessor.type) {
        case TINYGLTF_TYPE_SCALAR:
            typeCount = 1;
            break;
        case TINYGLTF_TYPE_VEC2:
            typeCount = 2;
            break;
        case TINYGLTF_TYPE_VEC3:
            typeCount = 3;
            break;
        case TINYGLTF_TYPE_VEC4:
            typeCount = 4;
            break;
        default:
            break;
    }
    return componentSize * typeCount;
}

const uint8_t* getBufferPointer(const tinygltf::Model& model, int accessorIdx) {
    const auto& accessor = model.accessors[accessorIdx];
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];
    return buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
}

GltfMesh processPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive) {
    GltfMesh mesh;
    mesh.vertices = std::make_shared<GltfMesh::VertexVector>();
    mesh.indices = std::make_shared<GltfMesh::TriangleIndexVector>();

    if (primitive.mode != TINYGLTF_MODE_TRIANGLES && primitive.mode != -1) {
        Log::Warning(Event::General, "glTF: unsupported primitive mode, only TRIANGLES supported");
        return mesh;
    }

    auto posIt = primitive.attributes.find("POSITION");
    if (posIt == primitive.attributes.end()) {
        Log::Warning(Event::General, "glTF: primitive missing POSITION attribute");
        return mesh;
    }

    const int posAccessorIdx = posIt->second;
    const size_t vertexCount = getAccessorCount(model, posAccessorIdx);
    const int posStride = getAccessorStride(model, posAccessorIdx);
    const uint8_t* posData = getBufferPointer(model, posAccessorIdx);

    // Texture coordinates (optional)
    const uint8_t* texData = nullptr;
    int texStride = 0;
    auto texIt = primitive.attributes.find("TEXCOORD_0");
    if (texIt != primitive.attributes.end()) {
        texData = getBufferPointer(model, texIt->second);
        texStride = getAccessorStride(model, texIt->second);
    }

    for (size_t i = 0; i < vertexCount; ++i) {
        GltfMesh::GeometryVertex vertex{};

        const auto* pos = reinterpret_cast<const float*>(posData + i * posStride);
        // Models use -Y as up; convert to Z-up for the map renderer:
        // (x, y, z) → (x, z, -y)
        vertex.position = {pos[0], pos[2], -pos[1]};

        if (texData) {
            const auto* tex = reinterpret_cast<const float*>(texData + i * texStride);
            vertex.texcoords = {tex[0], tex[1]};
        } else {
            vertex.texcoords = {0.0f, 0.0f};
        }

        mesh.vertices->emplace_back(std::move(vertex));
    }

    // Indices
    if (primitive.indices >= 0) {
        const auto& accessor = model.accessors[primitive.indices];
        const size_t indexCount = accessor.count;
        const uint8_t* indexData = getBufferPointer(model, primitive.indices);

        for (size_t i = 0; i + 2 < indexCount; i += 3) {
            uint16_t i0, i1, i2;
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const auto* data = reinterpret_cast<const uint16_t*>(indexData);
                i0 = data[i];
                i1 = data[i + 1];
                i2 = data[i + 2];
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                const auto* data = reinterpret_cast<const uint32_t*>(indexData);
                i0 = static_cast<uint16_t>(data[i]);
                i1 = static_cast<uint16_t>(data[i + 1]);
                i2 = static_cast<uint16_t>(data[i + 2]);
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                i0 = indexData[i];
                i1 = indexData[i + 1];
                i2 = indexData[i + 2];
            } else {
                continue;
            }
            mesh.indices->emplace_back(i0, i1, i2);
        }
    } else {
        // Non-indexed: generate sequential indices
        for (uint16_t i = 0; i + 2 < static_cast<uint16_t>(vertexCount); i += 3) {
            mesh.indices->emplace_back(i, static_cast<uint16_t>(i + 1), static_cast<uint16_t>(i + 2));
        }
    }

    return mesh;
}

std::optional<PremultipliedImage> extractTexture(const tinygltf::Model& model) {
    if (model.textures.empty() || model.images.empty()) {
        return std::nullopt;
    }

    // Use the first texture's image
    const auto& texture = model.textures[0];
    if (texture.source < 0 || texture.source >= static_cast<int>(model.images.size())) {
        return std::nullopt;
    }

    const auto& image = model.images[texture.source];
    if (image.image.empty() || image.width <= 0 || image.height <= 0) {
        return std::nullopt;
    }

    const auto width = static_cast<uint32_t>(image.width);
    const auto height = static_cast<uint32_t>(image.height);

    UnassociatedImage unassociated({width, height});

    if (image.component == 4) {
        assert(image.image.size() >= unassociated.bytes());
        std::copy(image.image.begin(), image.image.begin() + unassociated.bytes(), unassociated.data.get());
    } else if (image.component == 3) {
        // Convert RGB to RGBA
        auto* dst = unassociated.data.get();
        const auto* src = image.image.data();
        for (uint32_t i = 0; i < width * height; ++i) {
            dst[i * 4 + 0] = src[i * 3 + 0];
            dst[i * 4 + 1] = src[i * 3 + 1];
            dst[i * 4 + 2] = src[i * 3 + 2];
            dst[i * 4 + 3] = 255;
        }
    } else {
        Log::Warning(Event::General, "glTF: texture has unsupported component count");
        return std::nullopt;
    }

    return util::premultiply(std::move(unassociated));
}

GltfModel loadFromTinyGltfModel(const tinygltf::Model& model) {
    GltfModel result;

    for (const auto& gltfMesh : model.meshes) {
        for (const auto& primitive : gltfMesh.primitives) {
            auto mesh = processPrimitive(model, primitive);
            if (mesh.vertices && mesh.vertices->elements() > 0) {
                result.meshes.push_back(std::move(mesh));
            }
        }
    }

    result.texture = extractTexture(model);
    return result;
}

} // anonymous namespace

GltfModel loadGltf(const std::string& path) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool success = false;

    // Try binary first based on extension
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".glb") {
        success = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    } else {
        success = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    }

    if (!warn.empty()) {
        Log::Warning(Event::General, "glTF warning: " + warn);
    }

    if (!err.empty()) {
        Log::Error(Event::General, "glTF error: " + err);
    }

    if (!success) {
        Log::Error(Event::General, "Failed to load glTF file: " + path);
        return {};
    }

    return loadFromTinyGltfModel(model);
}

GltfModel loadGltfFromMemory(const std::string& data, const std::string& baseDir) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool success = false;

    // Try binary (GLB) first
    if (data.size() >= 4 && data[0] == 'g' && data[1] == 'l' && data[2] == 'T' && data[3] == 'F') {
        success = loader.LoadBinaryFromMemory(
            &model, &err, &warn, reinterpret_cast<const unsigned char*>(data.data()),
            static_cast<unsigned int>(data.size()), baseDir);
    } else {
        success = loader.LoadASCIIFromString(&model, &err, &warn, data.c_str(),
                                              static_cast<unsigned int>(data.size()), baseDir);
    }

    if (!warn.empty()) {
        Log::Warning(Event::General, "glTF warning: " + warn);
    }

    if (!err.empty()) {
        Log::Error(Event::General, "glTF error: " + err);
    }

    if (!success) {
        Log::Error(Event::General, "Failed to load glTF from memory");
        return {};
    }

    return loadFromTinyGltfModel(model);
}

} // namespace util
} // namespace mbgl
