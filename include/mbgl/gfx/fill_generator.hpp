#pragma once

#include <mbgl/util/geometry.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/programs/fill_program.hpp>

namespace mbgl {
namespace gfx {

void generateFillBuffers(const GeometryCollection& geometry,
                  gfx::VertexVector<FillLayoutVertex>& vertices,
                  SegmentVector<FillAttributes>& triangleSegments,
                  gfx::IndexVector<Triangles>& triangles);

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                            gfx::VertexVector<FillLayoutVertex>& vertices,
                            SegmentVector<FillAttributes>& lineSegments,
                            gfx::IndexVector<gfx::Lines>& lines,
                            SegmentVector<FillAttributes>& triangleSegments,
                            gfx::IndexVector<gfx::Triangles>& triangles);

} // namespace gfx
} // namespace mbgl
