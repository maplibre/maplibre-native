#include <mln/test/util.hpp>

#include <mln/util/constants.hpp>
#include <mln/util/subdivision.hpp>
#include <mln/util/subdivision_granularity.hpp>
#include <mln/util/tile_mesh.hpp>

#include <map>
#include <set>

using namespace mln;
using namespace mln::util;

namespace {

GeometryCollection square(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    return {{{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}, {x0, y0}}};
}

template <typename Index>
double signedArea(const std::vector<int16_t>& v, Index i0, Index i1, Index i2) {
    const double e0x = v[i1 * 2] - v[i0 * 2];
    const double e0y = v[i1 * 2 + 1] - v[i0 * 2 + 1];
    const double e1x = v[i2 * 2] - v[i0 * 2];
    const double e1y = v[i2 * 2 + 1] - v[i0 * 2 + 1];
    return e0x * e1y - e0y * e1x;
}

double totalArea(const SubdivisionResult& r) {
    double area = 0;
    for (std::size_t i = 0; i + 2 < r.triangleIndices.size(); i += 3) {
        area += std::abs(
                    signedArea(r.vertices, r.triangleIndices[i], r.triangleIndices[i + 1], r.triangleIndices[i + 2])) /
                2;
    }
    return area;
}

using Edge = std::pair<uint32_t, uint32_t>;

std::map<Edge, int> edgeCounts(const SubdivisionResult& r) {
    std::map<Edge, int> edges;
    for (std::size_t i = 0; i + 2 < r.triangleIndices.size(); i += 3) {
        for (std::size_t k = 0; k < 3; k++) {
            const uint32_t a = r.triangleIndices[i + k];
            const uint32_t b = r.triangleIndices[i + (k + 1) % 3];
            edges[{std::min(a, b), std::max(a, b)}]++;
        }
    }
    return edges;
}

// Every triangle with an edge on the tile's polar side gets its own quad up to the pole, and a polygon that reaches
// past that side has triangles on both sides of it, so edges into a pole and edges along that side are shared more
// than twice and are not part of the outline.
bool touchesPole(const SubdivisionResult& r, const Edge& edge) {
    const int16_t y0 = r.vertices[edge.first * 2 + 1];
    const int16_t y1 = r.vertices[edge.second * 2 + 1];
    const auto atPole = [](int16_t y) {
        return y == NORTH_POLE_Y || y == SOUTH_POLE_Y;
    };
    if (atPole(y0) || atPole(y1)) return true;
    if (y0 != y1) return false;
    const auto hasVertexAt = [&](int16_t pole) {
        for (std::size_t i = 1; i < r.vertices.size(); i += 2) {
            if (r.vertices[i] == pole) return true;
        }
        return false;
    };
    return (y0 == 0 && hasVertexAt(NORTH_POLE_Y)) || (y0 == EXTENT && hasVertexAt(SOUTH_POLE_Y));
}

// The checks GL JS `subdivision.test.ts` runs on every polygon: no vertex twice, every interior edge shared by exactly
// two triangles, no T-junctions, and the line lists are exactly the exposed edges of the mesh. At zoom 0 the
// triangles left and right of the tile are dropped, which exposes cut edges that were never outline, so that
// comparison is skipped for a clipped mesh, and for one built without outlines.
void expectSoundMesh(const SubdivisionResult& r, bool clippedAtZoomZero = false) {
    std::set<std::pair<int16_t, int16_t>> seen;
    for (std::size_t i = 0; i + 1 < r.vertices.size(); i += 2) {
        EXPECT_TRUE(seen.insert({r.vertices[i], r.vertices[i + 1]}).second) << "duplicate vertex " << i / 2;
    }

    const auto edges = edgeCounts(r);
    for (const auto& [edge, count] : edges) {
        if (count > 2 && !touchesPole(r, edge)) {
            ADD_FAILURE() << "edge (" << r.vertices[edge.first * 2] << ", " << r.vertices[edge.first * 2 + 1] << ")-("
                          << r.vertices[edge.second * 2] << ", " << r.vertices[edge.second * 2 + 1] << ") is shared by "
                          << count << " triangles";
        }
    }

    // T-junctions show up as boundary edges with a vertex lying strictly inside them.
    std::size_t tJunctions = 0;
    for (const auto& [edge, count] : edges) {
        if (count != 1) continue;
        const double ax = r.vertices[edge.first * 2], ay = r.vertices[edge.first * 2 + 1];
        const double bx = r.vertices[edge.second * 2], by = r.vertices[edge.second * 2 + 1];
        for (std::size_t v = 0; v < r.vertices.size() / 2; v++) {
            if (v == edge.first || v == edge.second) continue;
            const double px = r.vertices[v * 2], py = r.vertices[v * 2 + 1];
            const double cross = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
            if (cross != 0) continue;
            const double dot = (px - ax) * (bx - ax) + (py - ay) * (by - ay);
            const double len2 = (bx - ax) * (bx - ax) + (py - ay) * (by - ay);
            if (dot > 0 && dot < len2) {
                tJunctions++;
                break;
            }
        }
    }
    EXPECT_EQ(0u, tJunctions);

    if (clippedAtZoomZero || r.lineIndexLists.empty()) {
        return;
    }
    std::set<Edge> exposed;
    for (const auto& [edge, count] : edges) {
        if (count == 1 && !touchesPole(r, edge)) {
            exposed.insert(edge);
        }
    }
    std::set<Edge> outline;
    for (const auto& lines : r.lineIndexLists) {
        for (std::size_t i = 0; i + 1 < lines.size(); i += 2) {
            const Edge edge{std::min(lines[i], lines[i + 1]), std::max(lines[i], lines[i + 1])};
            if (touchesPole(r, edge)) continue;
            EXPECT_TRUE(outline.insert(edge).second) << "outline edge " << edge.first << "-" << edge.second << " twice";
        }
    }
    EXPECT_EQ(exposed, outline);
}

} // namespace

TEST(Subdivision, Granularity) {
    const SubdivisionGranularityExpression fill{128, 2};
    EXPECT_EQ(128u, fill.getGranularityForZoomLevel(0));
    EXPECT_EQ(64u, fill.getGranularityForZoomLevel(1));
    EXPECT_EQ(2u, fill.getGranularityForZoomLevel(6));
    EXPECT_EQ(2u, fill.getGranularityForZoomLevel(12));
    EXPECT_EQ(1u, SubdivisionGranularityExpression(0, 0).getGranularityForZoomLevel(3));
}

TEST(Subdivision, NoSubdivisionKeepsTriangles) {
    // GL JS "Polygon is unchanged when granularity=1".
    const auto result = subdividePolygon(square(0, 0, 20000, 20000), CanonicalTileID(3, 1, 1), 1);
    EXPECT_EQ((std::vector<int16_t>{0, 0, 20000, 0, 20000, 20000, 0, 20000}), result.vertices);
    // Two triangles on one diagonal (earcut's pick), wound the same way as GL JS's [2, 0, 3, 2, 1, 0].
    EXPECT_EQ(6u, result.triangleIndices.size());
    EXPECT_DOUBLE_EQ(20000.0 * 20000.0, totalArea(result));
    for (std::size_t i = 0; i + 2 < result.triangleIndices.size(); i += 3) {
        EXPECT_LT(signedArea(result.vertices,
                             result.triangleIndices[i],
                             result.triangleIndices[i + 1],
                             result.triangleIndices[i + 2]),
                  0.0);
    }
    ASSERT_EQ(1u, result.lineIndexLists.size());
    EXPECT_EQ((std::vector<uint32_t>{0, 1, 1, 2, 2, 3, 3, 0}), result.lineIndexLists[0]);
    expectSoundMesh(result);
}

TEST(Subdivision, SplitsOnCellBoundariesAndKeepsArea) {
    const auto full = square(0, 0, EXTENT, EXTENT);
    const auto result = subdividePolygon(full, CanonicalTileID(3, 1, 1), 4);
    // A 4x4 grid: 25 unique vertices, 32 triangles.
    EXPECT_EQ(25u, result.vertices.size() / 2);
    EXPECT_EQ(32u * 3u, result.triangleIndices.size());
    EXPECT_DOUBLE_EQ(static_cast<double>(EXTENT) * EXTENT, totalArea(result));
    for (std::size_t i = 0; i + 2 < result.triangleIndices.size(); i += 3) {
        EXPECT_LT(signedArea(result.vertices,
                             result.triangleIndices[i],
                             result.triangleIndices[i + 1],
                             result.triangleIndices[i + 2]),
                  0.0);
    }
    ASSERT_EQ(1u, result.lineIndexLists.size());
    EXPECT_EQ(16u * 2u, result.lineIndexLists[0].size());
    expectSoundMesh(result);
}

TEST(Subdivision, PoleTilesGetPoleQuads) {
    const auto full = square(0, 0, EXTENT, EXTENT);
    const auto north = subdividePolygon(full, CanonicalTileID(1, 0, 0), 2);
    std::set<int16_t> ys;
    for (std::size_t i = 1; i < north.vertices.size(); i += 2) {
        ys.insert(north.vertices[i]);
    }
    EXPECT_TRUE(ys.contains(NORTH_POLE_Y));
    EXPECT_FALSE(ys.contains(SOUTH_POLE_Y));
    expectSoundMesh(north);

    const auto south = subdividePolygon(full, CanonicalTileID(1, 0, 1), 2);
    ys.clear();
    for (std::size_t i = 1; i < south.vertices.size(); i += 2) {
        ys.insert(south.vertices[i]);
    }
    EXPECT_TRUE(ys.contains(SOUTH_POLE_Y));
    EXPECT_FALSE(ys.contains(NORTH_POLE_Y));
    expectSoundMesh(south);

    const auto middle = subdividePolygon(full, CanonicalTileID(2, 1, 1), 2);
    for (std::size_t i = 1; i < middle.vertices.size(); i += 2) {
        EXPECT_NE(NORTH_POLE_Y, middle.vertices[i]);
        EXPECT_NE(SOUTH_POLE_Y, middle.vertices[i]);
    }
    expectSoundMesh(middle);
}

TEST(Subdivision, ZoomZeroDropsGeometryOutsideTheTile) {
    const auto wide = square(-2000, 1000, EXTENT + 2000, 2000);
    const auto result = subdividePolygon(wide, CanonicalTileID(0, 0, 0), 8);
    // A triangle with any vertex in the buffer is a wrapped copy of geometry the tile itself draws.
    for (const uint32_t index : result.triangleIndices) {
        const int16_t x = result.vertices[index * 2];
        EXPECT_GE(x, 0);
        EXPECT_LE(x, EXTENT);
    }
    for (const auto& line : result.lineIndexLists) {
        for (const uint32_t index : line) {
            const int16_t x = result.vertices[index * 2];
            EXPECT_GE(x, 0);
            EXPECT_LE(x, EXTENT);
        }
    }
    expectSoundMesh(result, true);
}

TEST(Subdivision, VertexLine) {
    const GeometryCoordinates line{{0, 0}, {EXTENT, 0}};
    EXPECT_EQ(line, subdivideVertexLine(line, 1));
    const auto split = subdivideVertexLine(line, 4);
    ASSERT_EQ(5u, split.size());
    EXPECT_EQ(GeometryCoordinate(EXTENT / 4, 0), split[1]);
    EXPECT_EQ(GeometryCoordinate(EXTENT, 0), split.back());

    const GeometryCoordinates diagonal{{0, 0}, {EXTENT, EXTENT}};
    const auto diagonalSplit = subdivideVertexLine(diagonal, 2);
    ASSERT_EQ(3u, diagonalSplit.size());
    EXPECT_EQ(GeometryCoordinate(EXTENT / 2, EXTENT / 2), diagonalSplit[1]);

    const GeometryCoordinates ring{{0, 0}, {EXTENT, 0}, {EXTENT, EXTENT}};
    const auto closed = subdivideVertexLine(ring, 1, true);
    EXPECT_EQ(ring.front(), closed.back());
    EXPECT_EQ(4u, closed.size());
}

TEST(Subdivision, WorldPolygonWithBufferIsSound) {
    constexpr int16_t buffer = 512;
    const auto world = square(-buffer, -buffer, EXTENT + buffer, EXTENT + buffer);
    for (const uint8_t z : {0, 1}) {
        const auto result = subdividePolygon(world, CanonicalTileID(z, 0, 0), 128u >> z);
        expectSoundMesh(result, z == 0);
    }
}

TEST(Subdivision, PolygonBeyondTheIndexLimitCoarsens) {
    // A comb of 400 teeth hanging from a thin bar: 800 vertical edges each crossing 127 cell rows need more vertices
    // than a 16-bit index space holds; the fallback halves the granularity until the polygon fits rather than
    // dropping it.
    GeometryCoordinates ring;
    constexpr int16_t teeth = 400;
    constexpr int16_t step = EXTENT / (teeth * 2);
    constexpr int16_t bar = EXTENT - step;
    for (int16_t i = 0; i < teeth; ++i) {
        const auto x0 = static_cast<int16_t>(i * 2 * step);
        const auto x1 = static_cast<int16_t>(x0 + step);
        ring.emplace_back(x0, bar);
        ring.emplace_back(x0, 0);
        ring.emplace_back(x1, 0);
        ring.emplace_back(x1, bar);
    }
    ring.emplace_back(EXTENT, bar);
    ring.emplace_back(EXTENT, EXTENT);
    ring.emplace_back(0, EXTENT);
    ring.emplace_back(0, bar);
    const GeometryCollection polygon{ring};
    const CanonicalTileID tile(3, 4, 4);
    constexpr std::size_t limit = std::numeric_limits<uint16_t>::max();

    const auto full = subdividePolygon(polygon, tile, 128, false);
    EXPECT_GT(full.vertices.size() / 2, limit);

    // 800 edges crossing the cell rows: 128 and 64 both overflow, 32 is the first granularity that fits.
    EXPECT_GT(subdividePolygon(polygon, tile, 64, false).vertices.size() / 2, limit);
    const auto coarse = subdividePolygon(polygon, tile, 32, false);
    EXPECT_LE(coarse.vertices.size() / 2, limit);

    const auto fitted = subdividePolygonWithinLimit(polygon, tile, 128, false, limit);
    EXPECT_EQ(coarse.vertices, fitted.vertices);
    EXPECT_EQ(coarse.triangleIndices, fitted.triangleIndices);
    EXPECT_EQ(coarse.lineIndexLists, fitted.lineIndexLists);
    // The comb is about the vertex count; earcut's triangulation of it is not what is under test here.
}

TEST(TileMesh, QuadAndGrid) {
    const auto quad = createTileMesh({.granularity = 1});
    EXPECT_EQ(4u, quad.vertices.size() / 2);
    EXPECT_EQ(6u, quad.indices.size());

    const auto grid = createTileMesh({.granularity = 4, .generateBorders = true});
    EXPECT_EQ(7u * 7u, grid.vertices.size() / 2);
    EXPECT_EQ(6u * 6u * 6u, grid.indices.size());
    EXPECT_EQ(-EXTENT / 128, grid.vertices[0]);
    EXPECT_EQ(EXTENT + EXTENT / 128, grid.vertices[(7 * 7 - 1) * 2]);

    const auto polar = createTileMesh({.granularity = 2, .extendToNorthPole = true, .extendToSouthPole = true});
    EXPECT_EQ(3u * 5u, polar.vertices.size() / 2);
    EXPECT_EQ(NORTH_POLE_Y, polar.vertices[1]);
    EXPECT_EQ(SOUTH_POLE_Y, polar.vertices[(3 * 5 - 1) * 2 + 1]);
    for (std::size_t i = 0; i + 2 < polar.indices.size(); i += 3) {
        EXPECT_LT(signedArea(polar.vertices, polar.indices[i], polar.indices[i + 1], polar.indices[i + 2]), 0.0);
    }
}
