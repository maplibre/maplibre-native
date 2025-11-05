#include <mbgl/gfx/fill_generator.hpp>
#include <mbgl/gfx/polyline_generator.hpp>

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
        vertices.emplace_back(FillBucket::layoutVertex(point));
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

void addFillIndices(SegmentVector& fillSegments,
                    gfx::IndexVector<gfx::Triangles>& fillIndexes,
                    const std::span<const uint32_t>& indices,
                    const std::size_t startVertices,
                    const std::size_t totalVertices) {
    const std::size_t nIndices = indices.size();
    assert(nIndices % 3 == 0);

    if (fillSegments.empty() ||
        fillSegments.back().vertexLength + totalVertices > std::numeric_limits<uint16_t>::max()) {
        fillSegments.emplace_back(startVertices, fillIndexes.elements());
    }

    auto& triangleSegment = fillSegments.back();
    assert(triangleSegment.vertexLength + totalVertices <= std::numeric_limits<uint16_t>::max());
    const auto triangleIndex = static_cast<uint16_t>(triangleSegment.vertexLength);

    for (std::size_t i = 0; i < nIndices; i += 3) {
        assert(
            std::max({triangleIndex + indices[i + 0], triangleIndex + indices[i + 1], triangleIndex + indices[i + 2]}) <
            triangleIndex + totalVertices);
        fillIndexes.emplace_back(
            triangleIndex + indices[i], triangleIndex + indices[i + 1], triangleIndex + indices[i + 2]);
    }

    triangleSegment.vertexLength += totalVertices;
    triangleSegment.indexLength += nIndices;
}

void addOutlineIndices(const std::size_t base,
                       const std::size_t nVertices,
                       SegmentVector& lineSegments,
                       gfx::IndexVector<gfx::Lines>& lineIndexes) {
    if (nVertices == 0) return;

    if (lineSegments.empty() || lineSegments.back().vertexLength + nVertices > std::numeric_limits<uint16_t>::max()) {
        lineSegments.emplace_back(base, lineIndexes.elements());
    }

    auto& lineSegment = lineSegments.back();
    assert(lineSegment.vertexLength <= std::numeric_limits<uint16_t>::max());
    const auto lineIndex = static_cast<uint16_t>(lineSegment.vertexLength);

    lineIndexes.emplace_back(static_cast<uint16_t>(lineIndex + nVertices - 1), lineIndex);
    for (std::size_t i = 1; i < nVertices; i++) {
        lineIndexes.emplace_back(static_cast<uint16_t>(lineIndex + i - 1), static_cast<uint16_t>(lineIndex + i));
    }

    lineSegment.vertexLength += nVertices;
    lineSegment.indexLength += nVertices * 2;
}

} // namespace

void generateFillBuffers(const GeometryCollection& geometry,
                         gfx::VertexVector<FillLayoutVertex>& fillVertices,
                         gfx::IndexVector<Triangles>& fillIndexes,
                         SegmentVector& fillSegments) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = totalVerticesCheck(polygon);
        std::size_t startVertices = fillVertices.elements();

        for (const auto& ring : polygon) {
            addRingVertices(fillVertices, ring);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments, fillIndexes, indices, startVertices, totalVertices);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& vertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  gfx::IndexVector<gfx::Lines>& lineIndexes,
                                  SegmentVector& lineSegments) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = totalVerticesCheck(polygon);
        std::size_t startVertices = vertices.elements();

        for (const auto& ring : polygon) {
            std::size_t base = vertices.elements();
            std::size_t nVertices = addRingVertices(vertices, ring);
            addOutlineIndices(base, nVertices, lineSegments, lineIndexes);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments, fillIndexes, indices, startVertices, totalVertices);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  gfx::VertexVector<LineLayoutVertex>& lineVertices,
                                  gfx::IndexVector<gfx::Triangles>& lineIndexes,
                                  SegmentVector& lineSegments) {
    gfx::PolylineGenerator<LineLayoutVertex, SegmentBase> lineGenerator(
        lineVertices,
        LineBucket::layoutVertex,
        lineSegments,
        [](std::size_t vertexOffset, std::size_t indexOffset) -> SegmentBase {
            return SegmentBase(vertexOffset, indexOffset);
        },
        [](auto& seg) -> SegmentBase& { return seg; },
        lineIndexes);

    gfx::PolylineGeneratorOptions lineOptions;
    lineOptions.type = FeatureType::Polygon;

    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = totalVerticesCheck(polygon);
        std::size_t startVertices = fillVertices.elements();

        for (const auto& ring : polygon) {
            addRingVertices(fillVertices, ring);
            lineGenerator.generate(ring, lineOptions);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments, fillIndexes, indices, startVertices, totalVertices);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::IndexVector<gfx::Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  gfx::VertexVector<LineLayoutVertex>& lineVertices,
                                  gfx::IndexVector<gfx::Triangles>& lineIndexes,
                                  SegmentVector& lineSegments,
                                  gfx::IndexVector<gfx::Lines>& basicLineIndexes,
                                  SegmentVector& basicLineSegments) {
    gfx::PolylineGenerator<LineLayoutVertex, SegmentBase> lineGenerator(
        lineVertices,
        LineBucket::layoutVertex,
        lineSegments,
        [](std::size_t vertexOffset, std::size_t indexOffset) -> SegmentBase {
            return SegmentBase(vertexOffset, indexOffset);
        },
        [](auto& seg) -> SegmentBase& { return seg; },
        lineIndexes);

    gfx::PolylineGeneratorOptions lineOptions;
    lineOptions.type = FeatureType::Polygon;

    // If we have pre-tessellated geometry, multi-polygons are tessellated
    // together, so we need to add them to the fill segment all at once.
    if (!geometry.getTriangles().empty()) {
        const std::size_t startVertices = fillVertices.elements();
        std::size_t totalVertices = 0;
        for (const auto& polygon : geometry) {
            totalVertices += polygon.size();
            addRingVertices(fillVertices, polygon);
        }
        addFillIndices(fillSegments, fillIndexes, geometry.getTriangles(), startVertices, totalVertices);
        return;
    }

    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        const std::size_t totalVertices = totalVerticesCheck(polygon);
        const std::size_t startVertices = fillVertices.elements();

        for (const auto& ring : polygon) {
            const std::size_t base = fillVertices.elements();
            const std::size_t nVertices = addRingVertices(fillVertices, ring);
            addOutlineIndices(base, nVertices, basicLineSegments, basicLineIndexes);
            lineGenerator.generate(ring, lineOptions);
        }

        // tessellate, if no triangles are provided
        std::vector<uint32_t> indices = mapbox::earcut(polygon);

        addFillIndices(fillSegments, fillIndexes, indices, startVertices, totalVertices);
    }
}

} // namespace gfx

} // namespace mbgl
