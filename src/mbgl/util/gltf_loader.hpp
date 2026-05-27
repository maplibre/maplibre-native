#pragma once

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/util/image.hpp>

#include <memory>
#include <string>
#include <vector>

namespace mbgl {
namespace util {

struct GltfMesh {
    using GeometryVertex = style::CustomDrawableLayerHost::Interface::GeometryVertex;
    using VertexVector = gfx::VertexVector<GeometryVertex>;
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;

    std::shared_ptr<VertexVector> vertices;
    std::shared_ptr<TriangleIndexVector> indices;
};

struct GltfModel {
    std::vector<GltfMesh> meshes;
    std::optional<PremultipliedImage> texture;

    bool valid() const { return !meshes.empty(); }
};

/// Load a glTF or GLB file from disk and convert all mesh primitives
/// to the GeometryVertex format used by CustomDrawableLayer::addGeometry().
/// Only static meshes with basic textures are supported.
GltfModel loadGltf(const std::string& path);

/// Load a glTF or GLB model from raw binary data.
GltfModel loadGltfFromMemory(const std::string& data, const std::string& baseDir = "");

} // namespace util
} // namespace mbgl
