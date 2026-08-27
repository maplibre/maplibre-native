#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/tile/geometry_tile_data.hpp>

#include <cstdint>
#include <vector>

namespace mln {
namespace util {

/// Pole vertices carry these Y coordinates; the globe vertex shader maps them to the poles.
constexpr int16_t NORTH_POLE_Y = -32768;
constexpr int16_t SOUTH_POLE_Y = 32767;

struct SubdivisionResult {
    /// Flattened `x, y, x, y, ...` positions; every index below refers into this buffer.
    std::vector<int16_t> vertices;
    std::vector<uint32_t> triangleIndices;
    /// One line list per input ring, as index pairs.
    std::vector<std::vector<uint32_t>> lineIndexLists;
};

/// Triangulates a polygon and splits the triangles on a grid of `granularity` cells per axis. Port of GL JS
/// `render/subdivision.ts`: earcut, then a scanline pass per cell row, then pole quads on the pole tiles and
/// removal of geometry outside the tile on zoom 0. Output triangles are counter-clockwise in tile space.
SubdivisionResult subdividePolygon(const GeometryCollection& polygon,
                                   const CanonicalTileID& canonical,
                                   uint32_t granularity,
                                   bool generateOutlineLines = true);

/// `subdividePolygon` at the finest granularity whose result fits in `maxVertices` (the 16-bit index space of a
/// segment), halving from `granularity` down to 2. A polygon that cannot fit even unsubdivided comes back as is.
SubdivisionResult subdividePolygonWithinLimit(const GeometryCollection& polygon,
                                              const CanonicalTileID& canonical,
                                              uint32_t granularity,
                                              bool generateOutlineLines,
                                              std::size_t maxVertices);

/// Inserts a vertex wherever the line crosses a cell boundary; `isRing` closes the line first.
GeometryCoordinates subdivideVertexLine(const GeometryCoordinates& line, uint32_t granularity, bool isRing = false);

/// Reorders every triangle to counter-clockwise winding in tile space.
std::vector<uint32_t> fixWindingOrder(const std::vector<int16_t>& vertices, const std::vector<uint32_t>& indices);

/// Triangulates an ordered convex ring of vertex indices by walking outward from its leftmost vertex.
void scanlineTriangulateVertexRing(const std::vector<int16_t>& vertices,
                                   const std::vector<uint32_t>& ring,
                                   std::vector<uint32_t>& outIndices);

} // namespace util
} // namespace mln
