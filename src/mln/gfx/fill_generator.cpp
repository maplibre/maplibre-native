#include <mln/gfx/fill_generator.hpp>
#include <mln/gfx/polyline_generator.hpp>

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
struct nth<0, mln::GeometryCoordinate> {
    static int64_t get(const mln::GeometryCoordinate& t) { return t.x; };
};

template <>
struct nth<1, mln::GeometryCoordinate> {
    static int64_t get(const mln::GeometryCoordinate& t) { return t.y; };
};
} // namespace util
} // namespace mapbox

namespace mln {
namespace gfx {

struct GeometryTooLongException : std::exception {};

namespace {

void addFillExternalIndices(SegmentVector& fillSegments,
                            gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                            const std::span<const uint32_t>& indices,
                            const std::size_t startVertices) {
    const std::size_t nInstances = indices.size() / 3;
    assert(nInstances * 3 == indices.size());

    if (fillSegments.empty()) {
        fillSegments.emplace_back(0, 0, 3, 3, 0, 0);
    }

    auto& triangleSegment = fillSegments.back();
    triangleSegment.instanceCount += nInstances;

    for (std::size_t i = 0; i < nInstances; i++) {
        fillTriangles.emplace_back(FillBucket::triangleInstance(startVertices + indices[i * 3],
                                                                true,
                                                                false,
                                                                startVertices + indices[i * 3 + 1],
                                                                false,
                                                                true,
                                                                startVertices + indices[i * 3 + 2],
                                                                true,
                                                                false));
    }
}

std::size_t addRingVertices(gfx::VertexVector<FillLayoutVertex>& fillVertices,
                            gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                            gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                            SegmentVector& fillSegments,
                            const GeometryCoordinates& ring,
                            std::vector<bool>& ignoredVertices) {
    std::size_t startVertices = fillVertices.elements();
    uint countVertices = static_cast<uint>(ring.size() - 1);
    if (countVertices < 3) {
        return 0;
    }
    for (uint i = 0; i < countVertices; i++) {
        uint prevIndex = (i + countVertices - 1) % countVertices;
        uint nextIndex = (i + 1) % countVertices;

        auto& prev = ring[prevIndex];
        auto& point = ring[i];
        auto& next = ring[nextIndex];

        auto a = prev - point;
        auto b = next - point;
        float cross = a.x * b.y - a.y * b.x;

        if (cross > 0) {
            std::vector<uint32_t> indices = {prevIndex, i, nextIndex};
            addFillExternalIndices(fillSegments, fillTriangles, indices, startVertices);
        }

        fillVertices.emplace_back(FillBucket::layoutVertex(point));
        fillNeighbors.emplace_back(FillBucket::neighborVertex(prevIndex - i, nextIndex - i));
        ignoredVertices.emplace_back(cross > 0);
    }
    fillVertices.emplace_back(FillBucket::layoutVertex(ring[countVertices]));
    fillNeighbors.emplace_back(FillBucket::neighborVertex(-1, -countVertices + 1));
    ignoredVertices.emplace_back(ignoredVertices[0]);
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
                    gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                    const std::span<const uint32_t>& indices,
                    const std::size_t startVertices,
                    std::vector<bool>& ignoredVertices) {
    const std::size_t nInstances = indices.size() / 3;
    assert(nInstances * 3 == indices.size());

    if (fillSegments.empty()) {
        fillSegments.emplace_back(0, 0, 3, 3, 0, 0);
    }

    auto& triangleSegment = fillSegments.back();
    triangleSegment.instanceCount += nInstances;

    for (std::size_t i = 0; i < nInstances; i++) {
        fillTriangles.emplace_back(FillBucket::triangleInstance(startVertices + indices[i * 3],
                                                                ignoredVertices[indices[i * 3]],
                                                                false,
                                                                startVertices + indices[i * 3 + 1],
                                                                ignoredVertices[indices[i * 3 + 1]],
                                                                false,
                                                                startVertices + indices[i * 3 + 2],
                                                                ignoredVertices[indices[i * 3 + 2]],
                                                                false));
    }
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
                         gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                         gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                         SegmentVector& fillSegments) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tessellation.
        limitHoles(polygon, 500);

        [[maybe_unused]] std::size_t totalVertices = totalVerticesCheck(polygon);
        std::size_t startVertices = fillVertices.elements();
        std::vector<bool> ignoredVertices;

        for (const auto& ring : polygon) {
            addRingVertices(fillVertices, fillNeighbors, fillTriangles, fillSegments, ring, ignoredVertices);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments, fillTriangles, indices, startVertices, ignoredVertices);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                                  gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                                  SegmentVector& fillSegments,
                                  gfx::IndexVector<gfx::Lines>& lineIndexes,
                                  SegmentVector& lineSegments) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tessellation.
        limitHoles(polygon, 500);

        [[maybe_unused]] std::size_t totalVertices = totalVerticesCheck(polygon);
        std::size_t startVertices = fillVertices.elements();
        std::vector<bool> ignoredVertices;

        for (const auto& ring : polygon) {
            std::size_t base = fillVertices.elements();
            std::size_t nVertices = addRingVertices(fillVertices, fillNeighbors, fillTriangles, fillSegments, ring, ignoredVertices);
            addOutlineIndices(base, nVertices, lineSegments, lineIndexes);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments, fillTriangles, indices, startVertices, ignoredVertices);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                                  gfx::VertexVector<FillTriangleInstance>& fillTriangles,
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
        // Optimize polygons with many interior rings for earcut tessellation.
        limitHoles(polygon, 500);

        [[maybe_unused]] std::size_t totalVertices = totalVerticesCheck(polygon);
        std::size_t startVertices = fillVertices.elements();
        std::vector<bool> ignoredVertices;

        for (const auto& ring : polygon) {
            addRingVertices(fillVertices, fillNeighbors, fillTriangles, fillSegments, ring, ignoredVertices);
            lineGenerator.generate(ring, lineOptions);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments, fillTriangles, indices, startVertices, ignoredVertices);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                                  gfx::VertexVector<FillTriangleInstance>& fillTriangles,
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
        std::vector<bool> ignoredVertices;
        for (const auto& polygon : geometry) {
            addRingVertices(fillVertices, fillNeighbors, fillTriangles, fillSegments, polygon, ignoredVertices);
        }
        addFillIndices(fillSegments, fillTriangles, geometry.getTriangles(), startVertices, ignoredVertices);
        return;
    }

    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tessellation.
        limitHoles(polygon, 500);

        [[maybe_unused]] const std::size_t totalVertices = totalVerticesCheck(polygon);
        const std::size_t startVertices = fillVertices.elements();
        std::vector<bool> ignoredVertices;

        for (const auto& ring : polygon) {
            const std::size_t base = fillVertices.elements();
            const std::size_t nVertices = addRingVertices(
                fillVertices, fillNeighbors, fillTriangles, fillSegments, ring, ignoredVertices);
            addOutlineIndices(base, nVertices, basicLineSegments, basicLineIndexes);
            lineGenerator.generate(ring, lineOptions);
        }

        // tessellate, if no triangles are provided
        std::vector<uint32_t> indices = mapbox::earcut(polygon);

        addFillIndices(fillSegments, fillTriangles, indices, startVertices, ignoredVertices);
    }
}

} // namespace gfx

} // namespace mln
