#pragma once

#include <mln/util/geometry.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/gfx/vertex_vector.hpp>
#include <mln/gfx/index_vector.hpp>
#include <mln/renderer/buckets/fill_bucket.hpp>
#include <mln/renderer/buckets/line_bucket.hpp>

namespace mln {
namespace gfx {

/// Generate fill buffers, without outline
void generateFillBuffers(const GeometryCollection& geometry,
                         gfx::VertexVector<FillLayoutVertex>& fillVertices,
                         gfx::IndexVector<Triangles>& fillIndexes,
                         SegmentVector& fillSegments);

/// Generate fill and outline buffers, with the outline composed of line primitives.
void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& vertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  gfx::IndexVector<gfx::Lines>& lineIndexes,
                                  SegmentVector& lineSegments);

/// Generate fill and outline buffers, where the outlines are built with triangle primitives
void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  gfx::VertexVector<LineLayoutVertex>& lineVertices,
                                  gfx::IndexVector<gfx::Triangles>& lineIndexes,
                                  SegmentVector& lineSegments);

/// Generate fill and outline buffers, where the outlines are built both with triangle primitives AND with simple lines
void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  gfx::VertexVector<LineLayoutVertex>& lineVertices,
                                  gfx::IndexVector<gfx::Triangles>& lineIndexes,
                                  SegmentVector& lineSegments,
                                  gfx::IndexVector<gfx::Lines>& basicLineIndexes,
                                  SegmentVector& basicLineSegments);

} // namespace gfx
} // namespace mln
