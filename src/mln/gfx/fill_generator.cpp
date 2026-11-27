#include <mln/gfx/fill_generator.hpp>
#include <mln/gfx/fill_large_mesh_arrays.hpp>
#include <mln/gfx/polyline_generator.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/math.hpp>
#include <mln/util/subdivision.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <mapbox/earcut.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <algorithm>
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

namespace rounded_corner {

void addFillIndices(SegmentVector& fillSegments,
                    gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                    const std::span<const uint32_t>& indices,
                    const std::vector<bool>& convexAngleVertices,
                    const std::size_t startVertices) {
    const std::size_t nInstances = indices.size() / 3;
    assert(nInstances * 3 == indices.size());

    if (fillSegments.empty()) {
        fillSegments.emplace_back(0, 0, 3, 3, 0, 0);
    }

    auto& triangleSegment = fillSegments.back();
    triangleSegment.instanceCount += nInstances;

    for (std::size_t i = 0; i < nInstances; i++) {
        std::array<uint32_t, 3> triangleIndices;
        std::array<bool, 3> enableRounding;
        for (int k = 0; k < 3; k++) {
            triangleIndices[k] = static_cast<uint32_t>(startVertices) + indices[i * 3 + k];
            enableRounding[k] = convexAngleVertices[indices[i * 3 + k]];
        }
        fillTriangles.emplace_back(FillBucket::triangleInstance(triangleIndices, enableRounding, false));
    }
}

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
        std::array<uint32_t, 3> triangleIndices;
        std::array<bool, 3> enableRounding;
        for (int k = 0; k < 3; k++) {
            triangleIndices[k] = static_cast<uint32_t>(startVertices) + indices[i * 3 + k];
            enableRounding[k] = (k == 1);
        }
        fillTriangles.emplace_back(FillBucket::triangleInstance(triangleIndices, enableRounding, true));
    }
}

std::size_t addRingVertices(gfx::VertexVector<FillLayoutVertex>& fillVertices,
                            gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                            gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                            SegmentVector& fillSegments,
                            const GeometryCoordinates& ring,
                            std::vector<bool>& convexAngleVertices) {
    if (ring.size() <= 3) {
        return 0;
    }
    std::size_t startVertices = fillVertices.elements();
    uint32_t countVertices = static_cast<uint32_t>(ring.size() - 1);
    for (uint32_t i = 0; i < countVertices; i++) {
        uint32_t prevIndex = (i + countVertices - 1) % countVertices;
        uint32_t nextIndex = (i + 1) % countVertices;

        auto& prev = ring[prevIndex];
        auto& point = ring[i];
        auto& next = ring[nextIndex];

        bool isConvexAngle = util::crossProduct(prev - point, next - point) < 0;
        if (!isConvexAngle) {
            std::vector<uint32_t> indices = {prevIndex, i, nextIndex};
            addFillExternalIndices(fillSegments, fillTriangles, indices, startVertices);
        }

        fillVertices.emplace_back(FillBucket::layoutVertex(point));
        fillNeighbors.emplace_back(FillBucket::neighborVertex(prevIndex - i, nextIndex - i));
        convexAngleVertices.emplace_back(isConvexAngle);
    }
    fillVertices.emplace_back(FillBucket::layoutVertex(ring[countVertices]));
    fillNeighbors.emplace_back(FillBucket::neighborVertex(-1, -countVertices + 1));
    convexAngleVertices.emplace_back(convexAngleVertices[0]);
    return ring.size();
}

} // namespace rounded_corner

namespace sharp_corner {

std::size_t addRingVertices(gfx::VertexVector<FillLayoutVertex>& vertices, const GeometryCoordinates& ring) {
    for (auto& point : ring) {
        vertices.emplace_back(FillBucket::layoutVertex(point));
    }
    return ring.size();
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

} // namespace sharp_corner

namespace {

std::size_t addRingVertices(gfx::VertexVector<FillLayoutVertex>& fillVertices,
                            gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                            gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                            SegmentVector& fillSegments,
                            const GeometryCoordinates& ring,
                            std::vector<bool>& convexAngleVertices,
                            bool fillRoundedCornerEnabled) {
    if (fillRoundedCornerEnabled) {
        return rounded_corner::addRingVertices(
            fillVertices, fillNeighbors, fillTriangles, fillSegments, ring, convexAngleVertices);
    } else {
        return sharp_corner::addRingVertices(fillVertices, ring);
    }
}

std::size_t totalVerticesCheck(const GeometryCollection& polygon, bool fillRoundedCornerEnabled) {
    std::size_t totalVertices = 0;
    for (const auto& ring : polygon) {
        if (fillRoundedCornerEnabled && ring.size() > std::numeric_limits<int16_t>::max())
            throw GeometryTooLongException();
        totalVertices += ring.size();
    }
    if (totalVertices > std::numeric_limits<uint16_t>::max()) throw GeometryTooLongException();
    return totalVertices;
}

void addFillIndices(SegmentVector& fillSegments,
                    gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                    gfx::IndexVector<gfx::Triangles>& fillIndexes,
                    const std::span<const uint32_t>& indices,
                    const std::vector<bool>& convexAngleVertices,
                    const std::size_t startVertices,
                    const std::size_t totalVertices,
                    bool fillRoundedCornerEnabled) {
    if (fillRoundedCornerEnabled) {
        rounded_corner::addFillIndices(fillSegments, fillTriangles, indices, convexAngleVertices, startVertices);
    } else {
        sharp_corner::addFillIndices(fillSegments, fillIndexes, indices, startVertices, totalVertices);
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

bool insideTileX(const GeometryCoordinate& point) {
    return point.x >= 0 && point.x <= util::EXTENT;
}

/// On zoom 0 the tile's buffer wraps around the planet onto the tile itself, so the outline keeps only the runs
/// of vertices inside the tile's X extent, the way the subdivided fill drops its triangles.
template <class Generator>
void generateOutline(Generator& generator,
                     const GeometryCoordinates& ring,
                     const CanonicalTileID& canonical,
                     const gfx::PolylineGeneratorOptions& ringOptions) {
    if (canonical.z != 0 || std::all_of(ring.begin(), ring.end(), insideTileX)) {
        generator.generate(ring, ringOptions);
        return;
    }
    gfx::PolylineGeneratorOptions runOptions = ringOptions;
    runOptions.type = FeatureType::LineString;
    GeometryCoordinates run;
    for (const auto& point : ring) {
        if (insideTileX(point)) {
            run.push_back(point);
            continue;
        }
        if (run.size() > 1) {
            generator.generate(run, runOptions);
        }
        run.clear();
    }
    if (run.size() > 1) {
        generator.generate(run, runOptions);
    }
}

void addSubdividedPolygon(const util::SubdivisionResult& subdivided,
                          gfx::VertexVector<FillLayoutVertex>& fillVertices,
                          gfx::IndexVector<gfx::Triangles>& fillIndexes,
                          SegmentVector& fillSegments,
                          gfx::IndexVector<gfx::Lines>& lineIndexes,
                          SegmentVector& lineSegments) {
    const MeshLines lines{.segments = lineSegments, .indexes = lineIndexes, .lists = subdivided.lineIndexLists};
    fillLargeMeshArrays(
        fillVertices,
        [](int16_t x, int16_t y) { return FillBucket::layoutVertex({x, y}); },
        fillSegments,
        fillIndexes,
        subdivided.vertices,
        subdivided.triangleIndices,
        &lines);
}

util::SubdivisionResult subdivideWithBorderStrips(const GeometryCollection& polygon,
                                                  const CanonicalTileID& canonical,
                                                  uint32_t granularity) {
    auto subdivided = util::subdividePolygon(polygon, canonical, granularity, /*generateOutlineLines=*/true);
    util::addTileBorderStrips(subdivided, canonical, util::TILE_BORDER_STRIP_WIDTH);
    return subdivided;
}

} // namespace

void generateFillBuffers(const GeometryCollection& geometry,
                         gfx::VertexVector<FillLayoutVertex>& fillVertices,
                         gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                         gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                         gfx::IndexVector<Triangles>& fillIndexes,
                         SegmentVector& fillSegments,
                         bool fillRoundedCornerEnabled) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tessellation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = totalVerticesCheck(polygon, fillRoundedCornerEnabled);
        std::size_t startVertices = fillVertices.elements();
        std::vector<bool> convexAngleVertices;

        for (const auto& ring : polygon) {
            addRingVertices(fillVertices,
                            fillNeighbors,
                            fillTriangles,
                            fillSegments,
                            ring,
                            convexAngleVertices,
                            fillRoundedCornerEnabled);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments,
                       fillTriangles,
                       fillIndexes,
                       indices,
                       convexAngleVertices,
                       startVertices,
                       totalVertices,
                       fillRoundedCornerEnabled);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                                  gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                                  gfx::IndexVector<Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  bool fillRoundedCornerEnabled,
                                  gfx::IndexVector<gfx::Lines>& lineIndexes,
                                  SegmentVector& lineSegments,
                                  const CanonicalTileID& canonical,
                                  uint32_t subdivisionGranularity) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tessellation.
        limitHoles(polygon, 500);

        if (util::GLOBE_ENABLED && subdivisionGranularity >= 2) {
            addSubdividedPolygon(subdivideWithBorderStrips(polygon, canonical, subdivisionGranularity),
                                 fillVertices,
                                 fillIndexes,
                                 fillSegments,
                                 lineIndexes,
                                 lineSegments);
            continue;
        }

        std::size_t totalVertices = totalVerticesCheck(polygon, fillRoundedCornerEnabled);
        std::size_t startVertices = fillVertices.elements();
        std::vector<bool> convexAngleVertices;

        for (const auto& ring : polygon) {
            std::size_t base = fillVertices.elements();
            std::size_t nVertices = addRingVertices(fillVertices,
                                                    fillNeighbors,
                                                    fillTriangles,
                                                    fillSegments,
                                                    ring,
                                                    convexAngleVertices,
                                                    fillRoundedCornerEnabled);
            addOutlineIndices(base, nVertices, lineSegments, lineIndexes);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments,
                       fillTriangles,
                       fillIndexes,
                       indices,
                       convexAngleVertices,
                       startVertices,
                       totalVertices,
                       fillRoundedCornerEnabled);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                                  gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                                  gfx::IndexVector<Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  bool fillRoundedCornerEnabled,
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

        std::size_t totalVertices = totalVerticesCheck(polygon, fillRoundedCornerEnabled);
        std::size_t startVertices = fillVertices.elements();
        std::vector<bool> convexAngleVertices;

        for (const auto& ring : polygon) {
            addRingVertices(fillVertices,
                            fillNeighbors,
                            fillTriangles,
                            fillSegments,
                            ring,
                            convexAngleVertices,
                            fillRoundedCornerEnabled);
            lineGenerator.generate(ring, lineOptions);
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);
        addFillIndices(fillSegments,
                       fillTriangles,
                       fillIndexes,
                       indices,
                       convexAngleVertices,
                       startVertices,
                       totalVertices,
                       fillRoundedCornerEnabled);
    }
}

void generateFillAndOutineBuffers(const GeometryCollection& geometry,
                                  gfx::VertexVector<FillLayoutVertex>& fillVertices,
                                  gfx::VertexVector<FillNeighborVertex>& fillNeighbors,
                                  gfx::VertexVector<FillTriangleInstance>& fillTriangles,
                                  gfx::IndexVector<Triangles>& fillIndexes,
                                  SegmentVector& fillSegments,
                                  bool fillRoundedCornerEnabled,
                                  gfx::VertexVector<LineLayoutVertex>& lineVertices,
                                  gfx::IndexVector<gfx::Triangles>& lineIndexes,
                                  SegmentVector& lineSegments,
                                  gfx::IndexVector<gfx::Lines>& basicLineIndexes,
                                  SegmentVector& basicLineSegments,
                                  const CanonicalTileID& canonical,
                                  uint32_t subdivisionGranularity) {
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
    // Those triangles are flat: the globe subdivides the polygon instead.
    if (!geometry.getTriangles().empty() && (!util::GLOBE_ENABLED || subdivisionGranularity < 2)) {
        const std::size_t startVertices = fillVertices.elements();
        std::size_t totalVertices = 0;
        std::vector<bool> convexAngleVertices;
        for (const auto& polygon : geometry) {
            totalVertices += polygon.size();
            addRingVertices(fillVertices,
                            fillNeighbors,
                            fillTriangles,
                            fillSegments,
                            polygon,
                            convexAngleVertices,
                            fillRoundedCornerEnabled);
        }
        addFillIndices(fillSegments,
                       fillTriangles,
                       fillIndexes,
                       geometry.getTriangles(),
                       convexAngleVertices,
                       startVertices,
                       totalVertices,
                       fillRoundedCornerEnabled);
        return;
    }

    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tessellation.
        limitHoles(polygon, 500);

        if (util::GLOBE_ENABLED && subdivisionGranularity >= 2) {
            for (const auto& ring : polygon) {
                generateOutline(lineGenerator,
                                util::subdivideVertexLine(ring, subdivisionGranularity, /*isRing=*/true),
                                canonical,
                                lineOptions);
            }
            addSubdividedPolygon(subdivideWithBorderStrips(polygon, canonical, subdivisionGranularity),
                                 fillVertices,
                                 fillIndexes,
                                 fillSegments,
                                 basicLineIndexes,
                                 basicLineSegments);
            continue;
        }

        const std::size_t totalVertices = totalVerticesCheck(polygon, fillRoundedCornerEnabled);
        const std::size_t startVertices = fillVertices.elements();
        std::vector<bool> convexAngleVertices;

        for (const auto& ring : polygon) {
            const std::size_t base = fillVertices.elements();
            const std::size_t nVertices = addRingVertices(fillVertices,
                                                          fillNeighbors,
                                                          fillTriangles,
                                                          fillSegments,
                                                          ring,
                                                          convexAngleVertices,
                                                          fillRoundedCornerEnabled);
            addOutlineIndices(base, nVertices, basicLineSegments, basicLineIndexes);
            lineGenerator.generate(ring, lineOptions);
        }

        // tessellate, if no triangles are provided
        std::vector<uint32_t> indices = mapbox::earcut(polygon);

        addFillIndices(fillSegments,
                       fillTriangles,
                       fillIndexes,
                       indices,
                       convexAngleVertices,
                       startVertices,
                       totalVertices,
                       fillRoundedCornerEnabled);
    }
}

} // namespace gfx

} // namespace mln
