#pragma once

#include <cstdint>
#include <vector>

namespace mln {
namespace util {

struct TileMeshOptions {
    /// Quads per axis; 1 is the plain tile quad.
    uint32_t granularity = 1;
    /// Adds a ring of quads `EXTENT / 128` units outside the tile, for stencil masks.
    bool generateBorders = false;
    /// Replaces the north border with geometry reaching the north pole vertex.
    bool extendToNorthPole = false;
    bool extendToSouthPole = false;

    bool operator==(const TileMeshOptions&) const = default;
};

struct TileMesh {
    /// Flattened `x, y` pairs in tile units.
    std::vector<int16_t> vertices;
    std::vector<uint16_t> indices;
};

/// A grid mesh covering one tile, optionally with borders and pole caps; port of GL JS `util/create_tile_mesh.ts`.
/// Triangles are counter-clockwise in tile space.
TileMesh createTileMesh(const TileMeshOptions&);

} // namespace util
} // namespace mln
