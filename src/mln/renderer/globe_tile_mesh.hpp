#pragma once

#include <mln/gfx/index_vector.hpp>
#include <mln/gfx/vertex_vector.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/renderer/tile_mask.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/subdivision.hpp>
#include <mln/util/subdivision_granularity.hpp>
#include <mln/util/tile_mesh.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

namespace mln {

/// The pole-capped globe grid for the parts of a tile its mask keeps, as position + texture-position vertices. Each
/// part is gridded like the tile it stands in for, so it meets that tile's neighbours edge to edge; texture positions
/// stay inside the tile so the pole rows sample the tile's edge.
template <typename Vertex, typename LayoutVertexFn>
struct GlobeTileMesh {
    std::shared_ptr<gfx::VertexVector<Vertex>> vertices;
    std::shared_ptr<gfx::IndexVector<gfx::Triangles>> indices;
    SegmentVector segments;

    GlobeTileMesh(const CanonicalTileID& canonical, const TileMask& mask, LayoutVertexFn layoutVertex)
        : vertices(std::make_shared<gfx::VertexVector<Vertex>>()),
          indices(std::make_shared<gfx::IndexVector<gfx::Triangles>>()) {
        for (const auto& part : mask) {
            const util::TileMesh mesh = util::createTileMesh(
                {.granularity = SubdivisionGranularitySetting::globe().tile.getGranularityForZoomLevel(
                     static_cast<uint8_t>(canonical.z + part.z)),
                 .generateBorders = false,
                 .extendToNorthPole = canonical.y == 0 && part.y == 0,
                 .extendToSouthPole = canonical.y == (1u << canonical.z) - 1 && part.y == (1u << part.z) - 1});
            const std::size_t vertexCount = mesh.vertices.size() / 2;
            if (segments.empty() || segments.back().vertexLength + vertexCount > std::numeric_limits<uint16_t>::max()) {
                segments.emplace_back(vertices->elements(), indices->elements());
            }
            auto& segment = segments.back();
            const std::size_t offset = segment.vertexLength;
            const int32_t extent = util::EXTENT >> part.z;
            for (std::size_t i = 0; i + 1 < mesh.vertices.size(); i += 2) {
                const int16_t vy = mesh.vertices[i + 1];
                const Point<int16_t> position{static_cast<int16_t>(part.x * extent + (mesh.vertices[i] >> part.z)),
                                              vy == util::NORTH_POLE_Y || vy == util::SOUTH_POLE_Y
                                                  ? vy
                                                  : static_cast<int16_t>(part.y * extent + (vy >> part.z))};
                const Point<uint16_t> texture{static_cast<uint16_t>(std::clamp<int32_t>(position.x, 0, util::EXTENT)),
                                              static_cast<uint16_t>(std::clamp<int32_t>(position.y, 0, util::EXTENT))};
                vertices->emplace_back(layoutVertex(position, texture));
            }
            for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
                indices->emplace_back(static_cast<uint16_t>(offset + mesh.indices[i]),
                                      static_cast<uint16_t>(offset + mesh.indices[i + 1]),
                                      static_cast<uint16_t>(offset + mesh.indices[i + 2]));
            }
            segment.vertexLength += vertexCount;
            segment.indexLength += mesh.indices.size();
        }
    }
};

/// The grids a layer has built so far, one per zoom, pole row and mask: the mesh depends on nothing else.
template <typename Vertex, typename LayoutVertexFn>
class GlobeTileMeshCache {
public:
    const GlobeTileMesh<Vertex, LayoutVertexFn>& get(const CanonicalTileID& canonical,
                                                     const TileMask& mask,
                                                     LayoutVertexFn layoutVertex) {
        auto key = std::make_tuple(canonical.z, canonical.y == 0, canonical.y == (1u << canonical.z) - 1, mask);
        auto it = meshes.find(key);
        if (it == meshes.end()) {
            // Tiles that are loading go through many masks; start over rather than keep every one of them. The
            // drawables share the meshes' buffers, so they keep theirs.
            if (meshes.size() >= maxMeshes) {
                meshes.clear();
            }
            it = meshes.emplace(std::move(key), GlobeTileMesh<Vertex, LayoutVertexFn>(canonical, mask, layoutVertex))
                     .first;
        }
        return it->second;
    }
    void clear() { meshes.clear(); }
    std::size_t size() const { return meshes.size(); }

private:
    static constexpr std::size_t maxMeshes = 64;
    std::map<std::tuple<uint8_t, bool, bool, TileMask>, GlobeTileMesh<Vertex, LayoutVertexFn>> meshes;
};

/// The same grid as raw `Short2` positions, for drawables built from raw vertex bytes.
struct RawGlobeTileMesh {
    std::vector<std::uint8_t> vertices;
    std::size_t vertexCount = 0;
    std::vector<uint16_t> indices;
    SegmentVector segments;
};

inline RawGlobeTileMesh rawGlobeTileMesh(
    const CanonicalTileID& canonical,
    bool generateBorders,
    const SubdivisionGranularityExpression& granularity = SubdivisionGranularitySetting::globe().tile) {
    const util::TileMesh mesh = util::createTileMesh(
        {.granularity = granularity.getGranularityForZoomLevel(canonical.z),
         .generateBorders = generateBorders,
         .extendToNorthPole = canonical.y == 0,
         .extendToSouthPole = canonical.y == (1u << canonical.z) - 1});
    RawGlobeTileMesh raw;
    raw.vertices.resize(mesh.vertices.size() * sizeof(int16_t));
    std::memcpy(raw.vertices.data(), mesh.vertices.data(), raw.vertices.size());
    raw.vertexCount = mesh.vertices.size() / 2;
    assert(raw.vertexCount <= std::numeric_limits<uint16_t>::max());
    raw.indices.assign(mesh.indices.begin(), mesh.indices.end());
    raw.segments.emplace_back(0, 0, raw.vertexCount, raw.indices.size());
    return raw;
}

} // namespace mln
