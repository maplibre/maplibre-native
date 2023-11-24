#pragma once

#include <mbgl/util/geometry.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/programs/fill_program.hpp>
#include <mbgl/programs/line_program.hpp>

namespace mbgl {
namespace gfx {

/// Generate fill buffers, without outline
void generateFillBuffers(const GeometryCollection& geometry,
                         gfx::VertexVector<FillLayoutVertex>& fillVertices,
                         gfx::IndexVector<Triangles>& fillIndexes,
                         SegmentVector<FillAttributes>& fillSegments);

/// Generate fill and outline buffers, with the outline composed of line primitives.
void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& vertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector<FillAttributes>& fillSegments,
                                  gfx::IndexVector<gfx::Lines>& lineIndexes,
                                  SegmentVector<FillAttributes>& lineSegments);

/// Generate fill and outline buffers, where the outlines are built with triangle primitives
void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector<FillAttributes>& fillSegments,
                                  gfx::VertexVector<LineLayoutVertex>& lineVertices,
                                  gfx::IndexVector<gfx::Triangles>& lineIndexes,
                                  SegmentVector<LineAttributes>& lineSegments);

/// Generate fill and outline buffers, where the outlines are built both with triangle primitives AND with simple lines
void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector<FillAttributes>& fillSegments,
                                  gfx::VertexVector<LineLayoutVertex>& lineVertices,
                                  gfx::IndexVector<gfx::Triangles>& lineIndexes,
                                  SegmentVector<LineAttributes>& lineSegments,
                                  gfx::IndexVector<gfx::Lines>& basicLineIndexes,
                                  SegmentVector<FillAttributes>& basicLineSegments);

} // namespace gfx
} // namespace mbgl
