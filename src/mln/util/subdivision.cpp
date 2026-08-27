#include <mln/util/subdivision.hpp>
#include <mln/util/constants.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <mapbox/earcut.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_map>

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
namespace util {

namespace {

class Subdivider {
public:
    Subdivider(uint32_t granularity_, const CanonicalTileID& canonical_)
        : canonical(canonical_),
          granularity(granularity_),
          cellSize(static_cast<double>(EXTENT) / granularity_) {}

    SubdivisionResult subdivide(const GeometryCollection& polygon, bool generateOutlineLines) {
        std::vector<GeometryCoordinate> flattened;
        for (const auto& ring : polygon) {
            for (const auto& point : ring) {
                flattened.push_back(point);
                vertexToIndex(point.x, point.y);
            }
        }

        std::vector<uint32_t> triangles = convertIndices(flattened, mapbox::earcut(polygon));
        triangles = subdivideTrianglesScanline(triangles);

        std::vector<std::vector<uint32_t>> lines;
        if (generateOutlineLines) {
            lines = generateOutline(polygon);
        }

        ensureNoPoleVertices();
        handlePoles(triangles);

        if (granularity >= 2 && canonical.z == 0) {
            triangles = removeTrianglesOutsideTileX(triangles);
            for (auto& line : lines) {
                line = removeLinesOutsideTileX(line);
            }
        }

        return {.vertices = std::move(vertices),
                .triangleIndices = std::move(triangles),
                .lineIndexLists = std::move(lines)};
    }

private:
    static uint32_t key(int32_t x, int32_t y) {
        return (static_cast<uint32_t>(x + 32768) << 16) | static_cast<uint32_t>(y + 32768);
    }

    uint32_t vertexToIndex(double x, double y) {
        if (x < -32768 || y < -32768 || x > 32767 || y > 32767) {
            throw std::out_of_range("Vertex coordinates are out of signed 16 bit integer range.");
        }
        const auto xInt = static_cast<int32_t>(std::lround(x));
        const auto yInt = static_cast<int32_t>(std::lround(y));
        const uint32_t k = key(xInt, yInt);
        if (const auto it = dictionary.find(k); it != dictionary.end()) {
            return it->second;
        }
        const auto index = static_cast<uint32_t>(vertices.size() / 2);
        dictionary.emplace(k, index);
        vertices.push_back(static_cast<int16_t>(xInt));
        vertices.push_back(static_cast<int16_t>(yInt));
        return index;
    }

    std::vector<uint32_t> convertIndices(const std::vector<GeometryCoordinate>& flattened,
                                         const std::vector<uint32_t>& indices) {
        std::vector<uint32_t> converted;
        converted.reserve(indices.size());
        for (const uint32_t index : indices) {
            converted.push_back(vertexToIndex(flattened[index].x, flattened[index].y));
        }
        return converted;
    }

    std::vector<uint32_t> subdivideTrianglesScanline(const std::vector<uint32_t>& inputIndices) {
        if (granularity < 2) {
            return fixWindingOrder(vertices, inputIndices);
        }

        std::vector<uint32_t> finalIndices;
        for (std::size_t primitive = 0; primitive + 2 < inputIndices.size(); primitive += 3) {
            const std::array<uint32_t, 3> triangleIndices{
                inputIndices[primitive], inputIndices[primitive + 1], inputIndices[primitive + 2]};
            std::array<double, 6> triangle{};
            double minX = std::numeric_limits<double>::infinity();
            double minY = minX;
            double maxX = -std::numeric_limits<double>::infinity();
            double maxY = maxX;
            for (std::size_t i = 0; i < 3; i++) {
                triangle[i * 2] = vertices[triangleIndices[i] * 2];
                triangle[i * 2 + 1] = vertices[triangleIndices[i] * 2 + 1];
                minX = std::min(minX, triangle[i * 2]);
                maxX = std::max(maxX, triangle[i * 2]);
                minY = std::min(minY, triangle[i * 2 + 1]);
                maxY = std::max(maxY, triangle[i * 2 + 1]);
            }
            if (minX == maxX || minY == maxY) {
                continue;
            }

            const auto cellXmin = static_cast<int32_t>(std::floor(minX / cellSize));
            const auto cellXmax = static_cast<int32_t>(std::ceil(maxX / cellSize));
            const auto cellYmin = static_cast<int32_t>(std::floor(minY / cellSize));
            const auto cellYmax = static_cast<int32_t>(std::ceil(maxY / cellSize));

            if (cellXmin == cellXmax && cellYmin == cellYmax) {
                finalIndices.insert(finalIndices.end(), triangleIndices.begin(), triangleIndices.end());
                continue;
            }

            for (int32_t cellRow = cellYmin; cellRow < cellYmax; cellRow++) {
                const std::vector<uint32_t> ring = generateVertexRingForCellRow(cellRow, triangle, triangleIndices);
                scanlineTriangulateVertexRing(vertices, ring, finalIndices);
            }
        }
        return finalIndices;
    }

    std::vector<uint32_t> generateVertexRingForCellRow(int32_t cellRow,
                                                       const std::array<double, 6>& triangle,
                                                       const std::array<uint32_t, 3>& triangleIndices) {
        const double cellRowYTop = cellRow * cellSize;
        const double cellRowYBottom = cellRowYTop + cellSize;
        std::vector<uint32_t> ring;

        for (std::size_t edge = 0; edge < 3; edge++) {
            const double aX = triangle[edge * 2];
            const double aY = triangle[edge * 2 + 1];
            const double bX = triangle[((edge + 1) * 2) % 6];
            const double bY = triangle[((edge + 1) * 2 + 1) % 6];
            const double cX = triangle[((edge + 2) * 2) % 6];
            const double cY = triangle[((edge + 2) * 2 + 1) % 6];
            const double dirX = bX - aX;
            const double dirY = bY - aY;
            const bool isParallelY = dirX == 0;
            const bool isParallelX = dirY == 0;

            const double tTop = (cellRowYTop - aY) / dirY;
            const double tBottom = (cellRowYBottom - aY) / dirY;
            const double tEnter = std::min(tTop, tBottom);
            const double tExit = std::max(tTop, tBottom);

            if ((!isParallelX && (tEnter >= 1 || tExit <= 0)) ||
                (isParallelX && (aY < cellRowYTop || aY > cellRowYBottom))) {
                if (bY >= cellRowYTop && bY <= cellRowYBottom) {
                    ring.push_back(triangleIndices[(edge + 1) % 3]);
                }
                continue;
            }

            if (!isParallelX && tEnter > 0) {
                ring.push_back(vertexToIndex(aX + dirX * tEnter, aY + dirY * tEnter));
            }

            const double enterX = aX + dirX * std::max(tEnter, 0.0);
            const double exitX = aX + dirX * std::min(tExit, 1.0);

            if (!isParallelY) {
                generateIntraEdgeVertices(ring, aX, aY, bX, bY, enterX, exitX);
            }

            if (!isParallelX && tExit < 1) {
                ring.push_back(vertexToIndex(aX + dirX * tExit, aY + dirY * tExit));
            }

            if (isParallelX || (bY >= cellRowYTop && bY <= cellRowYBottom)) {
                ring.push_back(triangleIndices[(edge + 1) % 3]);
            }

            if (!isParallelX && (bY <= cellRowYTop || bY >= cellRowYBottom)) {
                generateInterEdgeVertices(ring, aX, aY, bX, bY, cX, cY, exitX, cellRowYTop, cellRowYBottom);
            }
        }
        return ring;
    }

    void generateIntraEdgeVertices(
        std::vector<uint32_t>& ring, double aX, double aY, double bX, double bY, double enterX, double exitX) {
        const double dirX = bX - aX;
        const double dirY = bY - aY;
        const bool isParallelX = dirY == 0;
        const double leftX = isParallelX ? std::min(aX, bX) : std::min(enterX, exitX);
        const double rightX = isParallelX ? std::max(aX, bX) : std::max(enterX, exitX);
        const auto leftCellX = static_cast<int32_t>(std::floor(leftX / cellSize)) + 1;
        const auto rightCellX = static_cast<int32_t>(std::ceil(rightX / cellSize)) - 1;
        const bool leftToRight = isParallelX ? (aX < bX) : (enterX < exitX);

        const auto emit = [&](int32_t cellX) {
            const double x = cellX * cellSize;
            ring.push_back(vertexToIndex(x, aY + dirY * (x - aX) / dirX));
        };
        if (leftToRight) {
            for (int32_t cellX = leftCellX; cellX <= rightCellX; cellX++) {
                emit(cellX);
            }
        } else {
            for (int32_t cellX = rightCellX; cellX >= leftCellX; cellX--) {
                emit(cellX);
            }
        }
    }

    void generateInterEdgeVertices(std::vector<uint32_t>& ring,
                                   double aX,
                                   double aY,
                                   double bX,
                                   double bY,
                                   double cX,
                                   double cY,
                                   double exitX,
                                   double cellRowYTop,
                                   double cellRowYBottom) {
        const double dirY = bY - aY;
        const double dir2X = cX - bX;
        const double dir2Y = cY - bY;
        const double t2Top = (cellRowYTop - bY) / dir2Y;
        const double t2Bottom = (cellRowYBottom - bY) / dir2Y;
        const double t2Enter = std::min(t2Top, t2Bottom);
        const double t2Exit = std::max(t2Top, t2Bottom);
        const double enter2X = bX + dir2X * t2Enter;

        auto leftCellX = static_cast<int32_t>(std::floor(std::min(enter2X, exitX) / cellSize)) + 1;
        auto rightCellX = static_cast<int32_t>(std::ceil(std::max(enter2X, exitX) / cellSize)) - 1;
        bool leftToRight = exitX < enter2X;

        const bool isParallelX2 = dir2Y == 0;
        if (isParallelX2 && (cY == cellRowYTop || cY == cellRowYBottom)) {
            return;
        }

        if (isParallelX2 || t2Enter >= 1 || t2Exit <= 0) {
            const double dir3X = aX - cX;
            const double dir3Y = aY - cY;
            const double t3Top = (cellRowYTop - cY) / dir3Y;
            const double t3Bottom = (cellRowYBottom - cY) / dir3Y;
            const double t3Enter = std::min(t3Top, t3Bottom);
            const double enter3X = cX + dir3X * t3Enter;
            leftCellX = static_cast<int32_t>(std::floor(std::min(enter3X, exitX) / cellSize)) + 1;
            rightCellX = static_cast<int32_t>(std::ceil(std::max(enter3X, exitX) / cellSize)) - 1;
            leftToRight = exitX < enter3X;
        }

        const double boundaryY = dirY > 0 ? cellRowYBottom : cellRowYTop;
        if (leftToRight) {
            for (int32_t cellX = leftCellX; cellX <= rightCellX; cellX++) {
                ring.push_back(vertexToIndex(cellX * cellSize, boundaryY));
            }
        } else {
            for (int32_t cellX = rightCellX; cellX >= leftCellX; cellX--) {
                ring.push_back(vertexToIndex(cellX * cellSize, boundaryY));
            }
        }
    }

    std::vector<std::vector<uint32_t>> generateOutline(const GeometryCollection& polygon) {
        std::vector<std::vector<uint32_t>> lines;
        for (const auto& ring : polygon) {
            const GeometryCoordinates path = subdivideVertexLine(ring, granularity, true);
            std::vector<uint32_t> lineIndices;
            for (std::size_t i = 1; i < path.size(); i++) {
                lineIndices.push_back(vertexToIndex(path[i - 1].x, path[i - 1].y));
                lineIndices.push_back(vertexToIndex(path[i].x, path[i].y));
            }
            lines.push_back(std::move(lineIndices));
        }
        return lines;
    }

    void ensureNoPoleVertices() {
        for (std::size_t i = 1; i < vertices.size(); i += 2) {
            if (vertices[i] == NORTH_POLE_Y) {
                vertices[i] = NORTH_POLE_Y + 1;
            }
            if (vertices[i] == SOUTH_POLE_Y) {
                vertices[i] = SOUTH_POLE_Y - 1;
            }
        }
    }

    void handlePoles(std::vector<uint32_t>& triangles) {
        const bool north = canonical.y == 0;
        const bool south = canonical.y == (1u << canonical.z) - 1;
        if (north || south) {
            fillPoles(triangles, north, south);
        }
    }

    void generatePoleQuad(
        std::vector<uint32_t>& indices, uint32_t i0, uint32_t i1, int16_t v0x, int16_t v1x, int16_t poleY) {
        const bool flip = (v0x > v1x) != (poleY == NORTH_POLE_Y);
        const uint32_t p0 = vertexToIndex(v0x, poleY);
        const uint32_t p1 = vertexToIndex(v1x, poleY);
        if (flip) {
            indices.insert(indices.end(), {i0, i1, p0, i1, p1, p0});
        } else {
            indices.insert(indices.end(), {i1, i0, p0, p1, i1, p0});
        }
    }

    void fillPoles(std::vector<uint32_t>& indices, bool north, bool south) {
        constexpr int16_t northEdge = 0;
        constexpr int16_t southEdge = EXTENT;
        const std::size_t count = indices.size();
        for (std::size_t primitive = 2; primitive < count; primitive += 3) {
            const uint32_t i0 = indices[primitive - 2];
            const uint32_t i1 = indices[primitive - 1];
            const uint32_t i2 = indices[primitive];
            const int16_t v0x = vertices[i0 * 2];
            const int16_t v0y = vertices[i0 * 2 + 1];
            const int16_t v1x = vertices[i1 * 2];
            const int16_t v1y = vertices[i1 * 2 + 1];
            const int16_t v2x = vertices[i2 * 2];
            const int16_t v2y = vertices[i2 * 2 + 1];
            if (north) {
                if (v0y == northEdge && v1y == northEdge) generatePoleQuad(indices, i0, i1, v0x, v1x, NORTH_POLE_Y);
                if (v1y == northEdge && v2y == northEdge) generatePoleQuad(indices, i1, i2, v1x, v2x, NORTH_POLE_Y);
                if (v2y == northEdge && v0y == northEdge) generatePoleQuad(indices, i2, i0, v2x, v0x, NORTH_POLE_Y);
            }
            if (south) {
                if (v0y == southEdge && v1y == southEdge) generatePoleQuad(indices, i0, i1, v0x, v1x, SOUTH_POLE_Y);
                if (v1y == southEdge && v2y == southEdge) generatePoleQuad(indices, i1, i2, v1x, v2x, SOUTH_POLE_Y);
                if (v2y == southEdge && v0y == southEdge) generatePoleQuad(indices, i2, i0, v2x, v0x, SOUTH_POLE_Y);
            }
        }
    }

    bool vertexOutsideTileX(uint32_t index) const {
        const int16_t x = vertices[index * 2];
        return x < 0 || x > EXTENT;
    }

    std::vector<uint32_t> removeTrianglesOutsideTileX(const std::vector<uint32_t>& indices) const {
        std::vector<uint32_t> result;
        for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
            if (vertexOutsideTileX(indices[i]) && vertexOutsideTileX(indices[i + 1]) &&
                vertexOutsideTileX(indices[i + 2])) {
                continue;
            }
            result.insert(result.end(), {indices[i], indices[i + 1], indices[i + 2]});
        }
        return result;
    }

    std::vector<uint32_t> removeLinesOutsideTileX(const std::vector<uint32_t>& indices) const {
        std::vector<uint32_t> result;
        for (std::size_t i = 0; i + 1 < indices.size(); i += 2) {
            if (vertexOutsideTileX(indices[i]) && vertexOutsideTileX(indices[i + 1])) {
                continue;
            }
            result.insert(result.end(), {indices[i], indices[i + 1]});
        }
        return result;
    }

    std::vector<int16_t> vertices;
    std::unordered_map<uint32_t, uint32_t> dictionary;
    const CanonicalTileID canonical;
    const uint32_t granularity;
    const double cellSize;
};

} // namespace

SubdivisionResult subdividePolygon(const GeometryCollection& polygon,
                                   const CanonicalTileID& canonical,
                                   uint32_t granularity,
                                   bool generateOutlineLines) {
    return Subdivider(granularity, canonical).subdivide(polygon, generateOutlineLines);
}

SubdivisionResult subdividePolygonWithinLimit(const GeometryCollection& polygon,
                                              const CanonicalTileID& canonical,
                                              uint32_t granularity,
                                              bool generateOutlineLines,
                                              std::size_t maxVertices) {
    while (true) {
        SubdivisionResult result = subdividePolygon(polygon, canonical, granularity, generateOutlineLines);
        if (result.vertices.size() / 2 <= maxVertices || granularity < 2) {
            return result;
        }
        granularity /= 2;
    }
}

GeometryCoordinates subdivideVertexLine(const GeometryCoordinates& line, uint32_t granularity, bool isRing) {
    if (line.size() < 2) {
        return {};
    }

    const bool closeRing = isRing && line.front() != line.back();
    if (granularity < 2) {
        GeometryCoordinates result = line;
        if (closeRing) {
            result.push_back(line.front());
        }
        return result;
    }

    const double cellSize = std::floor(static_cast<double>(EXTENT) / granularity);
    GeometryCoordinates result;
    result.push_back(line.front());

    const auto pushUnique = [&](double x, double y) {
        const GeometryCoordinate next{static_cast<int16_t>(x), static_cast<int16_t>(y)};
        if (result.back() != next) {
            result.push_back(next);
        }
    };

    const std::size_t total = line.size();
    const std::size_t lastIndex = closeRing ? total : total - 1;
    for (std::size_t pointIndex = 0; pointIndex < lastIndex; pointIndex++) {
        const GeometryCoordinate& p0 = line[pointIndex];
        const GeometryCoordinate& p1 = pointIndex < total - 1 ? line[pointIndex + 1] : line[0];
        const double v0x = p0.x;
        const double v0y = p0.y;
        const double v1x = p1.x;
        const double v1y = p1.y;

        const bool dirXnonZero = v0x != v1x;
        const bool dirYnonZero = v0y != v1y;
        if (!dirXnonZero && !dirYnonZero) {
            continue;
        }

        const double dirX = v1x - v0x;
        const double dirY = v1y - v0y;
        const double absDirX = std::abs(dirX);
        const double absDirY = std::abs(dirY);
        double lastX = v0x;
        double lastY = v0y;

        while (true) {
            const double nextBoundaryX = dirX > 0 ? (std::floor(lastX / cellSize) + 1) * cellSize
                                                  : (std::ceil(lastX / cellSize) - 1) * cellSize;
            const double nextBoundaryY = dirY > 0 ? (std::floor(lastY / cellSize) + 1) * cellSize
                                                  : (std::ceil(lastY / cellSize) - 1) * cellSize;
            const double axisDistanceToBoundaryX = std::abs(lastX - nextBoundaryX);
            const double axisDistanceToBoundaryY = std::abs(lastY - nextBoundaryY);
            const double axisDistanceToEndX = std::abs(lastX - v1x);
            const double axisDistanceToEndY = std::abs(lastY - v1y);
            const double realDistanceToBoundaryX = dirXnonZero ? axisDistanceToBoundaryX / absDirX
                                                               : std::numeric_limits<double>::infinity();
            const double realDistanceToBoundaryY = dirYnonZero ? axisDistanceToBoundaryY / absDirY
                                                               : std::numeric_limits<double>::infinity();

            if ((axisDistanceToEndX <= axisDistanceToBoundaryX || !dirXnonZero) &&
                (axisDistanceToEndY <= axisDistanceToBoundaryY || !dirYnonZero)) {
                break;
            }

            if ((realDistanceToBoundaryX < realDistanceToBoundaryY && dirXnonZero) || !dirYnonZero) {
                lastX = nextBoundaryX;
                lastY = lastY + dirY * realDistanceToBoundaryX;
                pushUnique(lastX, std::round(lastY));
            } else {
                lastX = lastX + dirX * realDistanceToBoundaryY;
                lastY = nextBoundaryY;
                pushUnique(std::round(lastX), lastY);
            }
        }
        pushUnique(v1x, v1y);
    }
    return result;
}

std::vector<uint32_t> fixWindingOrder(const std::vector<int16_t>& vertices, const std::vector<uint32_t>& indices) {
    std::vector<uint32_t> corrected;
    corrected.reserve(indices.size());
    for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
        const uint32_t i0 = indices[i];
        const uint32_t i1 = indices[i + 1];
        const uint32_t i2 = indices[i + 2];
        const double e0x = vertices[i1 * 2] - vertices[i0 * 2];
        const double e0y = vertices[i1 * 2 + 1] - vertices[i0 * 2 + 1];
        const double e1x = vertices[i2 * 2] - vertices[i0 * 2];
        const double e1y = vertices[i2 * 2 + 1] - vertices[i0 * 2 + 1];
        if (e0x * e1y - e0y * e1x > 0) {
            corrected.insert(corrected.end(), {i0, i2, i1});
        } else {
            corrected.insert(corrected.end(), {i0, i1, i2});
        }
    }
    return corrected;
}

void scanlineTriangulateVertexRing(const std::vector<int16_t>& vertices,
                                   const std::vector<uint32_t>& ring,
                                   std::vector<uint32_t>& outIndices) {
    if (ring.empty()) {
        throw std::invalid_argument("Subdivision vertex ring is empty.");
    }

    std::size_t leftmostIndex = 0;
    int16_t leftmostX = vertices[ring[0] * 2];
    for (std::size_t i = 1; i < ring.size(); i++) {
        const int16_t x = vertices[ring[i] * 2];
        if (x < leftmostX) {
            leftmostX = x;
            leftmostIndex = i;
        }
    }

    const std::size_t length = ring.size();
    std::size_t lastEdgeA = leftmostIndex;
    std::size_t lastEdgeB = (lastEdgeA + 1) % length;

    while (true) {
        const std::size_t candidateA = lastEdgeA >= 1 ? lastEdgeA - 1 : length - 1;
        const std::size_t candidateB = (lastEdgeB + 1) % length;

        const double candidateAx = vertices[ring[candidateA] * 2];
        const double candidateAy = vertices[ring[candidateA] * 2 + 1];
        const double candidateBx = vertices[ring[candidateB] * 2];
        const double candidateBy = vertices[ring[candidateB] * 2 + 1];
        const double lastEdgeAx = vertices[ring[lastEdgeA] * 2];
        const double lastEdgeAy = vertices[ring[lastEdgeA] * 2 + 1];
        const double lastEdgeBx = vertices[ring[lastEdgeB] * 2];
        const double lastEdgeBy = vertices[ring[lastEdgeB] * 2 + 1];

        bool pickA = false;
        if (candidateAx < candidateBx) {
            pickA = true;
        } else if (candidateAx > candidateBx) {
            pickA = false;
        } else {
            const double nx = lastEdgeBy - lastEdgeAy;
            const double ny = -(lastEdgeBx - lastEdgeAx);
            const double sign = lastEdgeAy < lastEdgeBy ? 1 : -1;
            const double aRight = ((candidateAx - lastEdgeAx) * nx + (candidateAy - lastEdgeAy) * ny) * sign;
            const double bRight = ((candidateBx - lastEdgeAx) * nx + (candidateBy - lastEdgeAy) * ny) * sign;
            pickA = aRight > bRight;
        }

        const uint32_t a = ring[lastEdgeA];
        const uint32_t b = ring[lastEdgeB];
        if (pickA) {
            const uint32_t c = ring[candidateA];
            if (c != a && c != b && a != b) {
                outIndices.insert(outIndices.end(), {b, a, c});
            }
            lastEdgeA = lastEdgeA >= 1 ? lastEdgeA - 1 : length - 1;
        } else {
            const uint32_t c = ring[candidateB];
            if (c != a && c != b && a != b) {
                outIndices.insert(outIndices.end(), {b, a, c});
            }
            lastEdgeB = (lastEdgeB + 1) % length;
        }

        if (candidateA == candidateB) {
            break;
        }
    }
}

} // namespace util
} // namespace mln
