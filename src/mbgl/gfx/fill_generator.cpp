#include <mbgl/gfx/fill_generator.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <mapbox/earcut.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <cassert>
#include <limits>

namespace mapbox {
namespace util {
template <>
struct nth<0, mbgl::GeometryCoordinate> {
    static int64_t get(const mbgl::GeometryCoordinate& t) { return t.x; };
};

template <>
struct nth<1, mbgl::GeometryCoordinate> {
    static int64_t get(const mbgl::GeometryCoordinate& t) { return t.y; };
};
} // namespace util
} // namespace mapbox

namespace mbgl {
namespace gfx {

struct GeometryTooLongException : std::exception {};

namespace {

std::size_t addRingVertices(gfx::VertexVector<FillLayoutVertex>& vertices, const GeometryCoordinates& ring) {
    for (auto& point : ring) {
        vertices.emplace_back(FillProgram::layoutVertex(point));
    }
    return ring.size();
}

std::size_t totalVerticesCheck(const GeometryCollection& polygon) {
    std::size_t totalVertices = 0;
    for (const auto& ring : polygon) {
        totalVertices += ring.size();
    }
    if (totalVertices > std::numeric_limits<uint16_t>::max()) throw GeometryTooLongException();
    return totalVertices;
}

void addFillIndices(SegmentVector<FillAttributes>& triangleSegments,
                    gfx::IndexVector<gfx::Triangles>& triangles,
                    std::vector<uint32_t>& indices,
                    std::size_t startVertices,
                    std::size_t totalVertices) {
    std::size_t nIndices = indices.size();
    assert(nIndices % 3 == 0);
    
    if (triangleSegments.empty() ||
        triangleSegments.back().vertexLength + totalVertices > std::numeric_limits<uint16_t>::max()) {
        triangleSegments.emplace_back(startVertices, triangles.elements());
    }
    
    auto& triangleSegment = triangleSegments.back();
    assert(triangleSegment.vertexLength <= std::numeric_limits<uint16_t>::max());
    const auto triangleIndex = static_cast<uint16_t>(triangleSegment.vertexLength);
    
    for (std::size_t i = 0; i < nIndices; i += 3) {
        triangles.emplace_back(
                               triangleIndex + indices[i], triangleIndex + indices[i + 1], triangleIndex + indices[i + 2]);
    }
    
    triangleSegment.vertexLength += totalVertices;
    triangleSegment.indexLength += nIndices;
}

void addOutlineIndices(std::size_t base, std::size_t nVertices, SegmentVector<FillAttributes>& lineSegments, gfx::IndexVector<gfx::Lines>& lines) {
    if (nVertices == 0) return;
    
    if (lineSegments.empty() ||
        lineSegments.back().vertexLength + nVertices > std::numeric_limits<uint16_t>::max()) {
        lineSegments.emplace_back(base, lines.elements());
    }
    
    auto& lineSegment = lineSegments.back();
    assert(lineSegment.vertexLength <= std::numeric_limits<uint16_t>::max());
    const auto lineIndex = static_cast<uint16_t>(lineSegment.vertexLength);
    
    lines.emplace_back(static_cast<uint16_t>(lineIndex + nVertices - 1), lineIndex);
    for (std::size_t i = 1; i < nVertices; i++) {
        lines.emplace_back(static_cast<uint16_t>(lineIndex + i - 1), static_cast<uint16_t>(lineIndex + i));
    }
    
    lineSegment.vertexLength += nVertices;
    lineSegment.indexLength += nVertices * 2;
}

} // namespace

void generateFillBuffers(const GeometryCollection& geometry,
                  gfx::VertexVector<FillLayoutVertex>& vertices,
                  SegmentVector<FillAttributes>& triangleSegments,
                  gfx::IndexVector<gfx::Triangles>& triangles) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = totalVerticesCheck(polygon);
        std::size_t startVertices = vertices.elements();

        for (const auto& ring : polygon) {
            addRingVertices(vertices, ring);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(triangleSegments, triangles, indices, startVertices, totalVertices);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                            gfx::VertexVector<FillLayoutVertex>& vertices,
                            SegmentVector<FillAttributes>& lineSegments,
                            gfx::IndexVector<gfx::Lines>& lines,
                            SegmentVector<FillAttributes>& triangleSegments,
                            gfx::IndexVector<gfx::Triangles>& triangles) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = totalVerticesCheck(polygon);
        std::size_t startVertices = vertices.elements();

        for (const auto& ring : polygon) {
            std::size_t base = vertices.elements();
            std::size_t nVertices = addRingVertices(vertices, ring);
            addOutlineIndices(base, nVertices, lineSegments, lines);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(triangleSegments, triangles, indices, startVertices, totalVertices);
    }
}

} // namespace gfx

} // namespace mbgl
