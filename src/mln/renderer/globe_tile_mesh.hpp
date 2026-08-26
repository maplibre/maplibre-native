#pragma once

#include <mln/gfx/index_vector.hpp>
#include <mln/gfx/vertex_vector.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/subdivision_granularity.hpp>
#include <mln/util/tile_mesh.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

namespace mln {

/// The pole-capped globe grid for a tile, as position + texture-position vertices; texture positions stay inside
/// the tile so the pole rows sample the tile's edge.
template <typename Vertex, typename LayoutVertexFn>
struct GlobeTileMesh {
    std::shared_ptr<gfx::VertexVector<Vertex>> vertices;
    std::shared_ptr<gfx::IndexVector<gfx::Triangles>> indices;
    SegmentVector segments;

    GlobeTileMesh(const CanonicalTileID& canonical, LayoutVertexFn layoutVertex)
        : vertices(std::make_shared<gfx::VertexVector<Vertex>>()),
          indices(std::make_shared<gfx::IndexVector<gfx::Triangles>>()) {
        const util::TileMesh mesh = util::createTileMesh(
            {.granularity = SubdivisionGranularitySetting::globe().tile.getGranularityForZoomLevel(canonical.z),
             .generateBorders = false,
             .extendToNorthPole = canonical.y == 0,
             .extendToSouthPole = canonical.y == (1u << canonical.z) - 1});
        for (std::size_t i = 0; i + 1 < mesh.vertices.size(); i += 2) {
            const Point<int16_t> position{mesh.vertices[i], mesh.vertices[i + 1]};
            const Point<uint16_t> texture{static_cast<uint16_t>(std::clamp<int32_t>(position.x, 0, util::EXTENT)),
                                          static_cast<uint16_t>(std::clamp<int32_t>(position.y, 0, util::EXTENT))};
            vertices->emplace_back(layoutVertex(position, texture));
        }
        for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
            indices->emplace_back(mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]);
        }
        segments.emplace_back(0, 0, vertices->elements(), indices->elements());
    }
};

/// The grids a layer has built so far, one per zoom and pole row: the mesh depends on nothing else.
template <typename Vertex, typename LayoutVertexFn>
class GlobeTileMeshCache {
public:
    const GlobeTileMesh<Vertex, LayoutVertexFn>& get(const CanonicalTileID& canonical, LayoutVertexFn layoutVertex) {
        const auto key = std::make_tuple(canonical.z, canonical.y == 0, canonical.y == (1u << canonical.z) - 1);
        auto it = meshes.find(key);
        if (it == meshes.end()) {
            it = meshes.emplace(key, GlobeTileMesh<Vertex, LayoutVertexFn>(canonical, layoutVertex)).first;
        }
        return it->second;
    }

private:
    std::map<std::tuple<uint8_t, bool, bool>, GlobeTileMesh<Vertex, LayoutVertexFn>> meshes;
};

/// The same grid as raw `Short2` positions, for drawables built from raw vertex bytes.
struct RawGlobeTileMesh {
    std::vector<std::uint8_t> vertices;
    std::size_t vertexCount = 0;
    std::vector<uint16_t> indices;
    SegmentVector segments;
};

inline RawGlobeTileMesh rawGlobeTileMesh(const CanonicalTileID& canonical, bool generateBorders) {
    const util::TileMesh mesh = util::createTileMesh(
        {.granularity = SubdivisionGranularitySetting::globe().tile.getGranularityForZoomLevel(canonical.z),
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
