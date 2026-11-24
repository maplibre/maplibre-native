#include <mln/test/util.hpp>

#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/shaders/layer_ubo.hpp>
#include <mln/util/constants.hpp>

#include <mln/test/stub_geometry_tile_feature.hpp>

#include <array>
#include <cstdint>
#include <map>
#include <memory>

using namespace mln;

namespace {

// A polygon with one closed ring, as the tile worker hands it to the bucket.
GeometryCollection closedSquare(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    GeometryCollection polygon;
    polygon.push_back({{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}, {x0, y0}});
    return polygon;
}

std::unique_ptr<FillExtrusionBucket> buildBucket(GeometryCollection geometry) {
    FillExtrusionBucket::PossiblyEvaluatedLayoutProperties layout;
    auto bucket = std::make_unique<FillExtrusionBucket>(
        layout, std::map<std::string, Immutable<style::LayerProperties>>{}, 17.0f, 1);
    StubGeometryTileFeature feature{FeatureType::Polygon, std::move(geometry)};
    bucket->addFeature(feature, feature.getGeometries(), {}, {}, 0, CanonicalTileID{17, 0, 0});
    return bucket;
}

std::array<int16_t, 2> centroidOf(const FillExtrusionBucket& bucket, std::size_t i) {
    const auto& vertex = bucket.vertices.at(i);
#if MLN_USE_FILL_EXTRUSION_INSTANCING
    return {vertex.a3[0], vertex.a3[1]};
#else
    return {vertex.a4[0], vertex.a4[1]};
#endif
}

// A square ring that runs the other way round, so that it is a hole in a polygon whose outer ring is a
// closedSquare.
GeometryCoordinates closedHole(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    return {{x0, y0}, {x0, y1}, {x1, y1}, {x1, y0}, {x0, y0}};
}

// The number of walls the bucket draws, given the number of ring vertices it was given. A wall takes four
// vertices of its own, except with instancing, where every ring vertex is one wall that is either drawn or
// marked as discarded.
std::size_t wallCount(const FillExtrusionBucket& bucket, [[maybe_unused]] std::size_t ringVertices) {
#if MLN_USE_FILL_EXTRUSION_INSTANCING
    std::size_t walls = 0;
    for (std::size_t i = 0; i < bucket.vertices.elements(); ++i) {
        // The last vertex of a ring has no wall, and is marked as discarded too.
        if ((bucket.vertices.at(i).a2[0] & 1) == 0) ++walls;
    }
    return walls;
#else
    return (bucket.vertices.elements() - ringVertices) / 4;
#endif
}

} // namespace

// The terrain lifts a whole extrusion by the elevation at one polygon centroid. A closed ring repeats its
// first vertex, and counting that twice pulled the centroid towards wherever the ring starts.
TEST(FillExtrusionBucket, CentroidIgnoresTheRepeatedClosingVertex) {
    const auto bucket = buildBucket(closedSquare(0, 0, 1000, 1000));

    ASSERT_GT(bucket->vertices.elements(), 0u);
    for (std::size_t i = 0; i < bucket->vertices.elements(); ++i) {
        EXPECT_EQ((std::array<int16_t, 2>{500, 500}), centroidOf(*bucket, i)) << "vertex " << i;
    }
}

// A copy of a polygon that lies wholly in the tile buffer is cut off at the buffer edge. Its centroid, and
// so its terrain elevation, differs from the copy in the tile that holds the polygon, and drawn together
// they step across the roof. Only the tile that holds it draws it.
TEST(FillExtrusionBucket, PolygonWhollyInTheBufferIsNotDrawn) {
    const auto extent = static_cast<int16_t>(util::EXTENT);
    EXPECT_EQ(0u, buildBucket(closedSquare(1000, extent + 100, 2000, extent + 2048))->vertices.elements());
    EXPECT_EQ(0u, buildBucket(closedSquare(1000, -2048, 2000, -100))->vertices.elements());
    EXPECT_EQ(0u, buildBucket(closedSquare(extent + 100, 1000, extent + 2048, 2000))->vertices.elements());
    EXPECT_EQ(0u, buildBucket(closedSquare(-2048, 1000, -100, 2000))->vertices.elements());
}

TEST(FillExtrusionBucket, PolygonReachingIntoTheTileIsDrawn) {
    const auto extent = static_cast<int16_t>(util::EXTENT);
    EXPECT_GT(buildBucket(closedSquare(1000, extent - 100, 2000, extent + 2048))->vertices.elements(), 0u);
    EXPECT_GT(buildBucket(closedSquare(1000, 1000, 2000, 2000))->vertices.elements(), 0u);
}

// maplibre-gl-js takes the centroid over every ring, holes included.
TEST(FillExtrusionBucket, CentroidTakesTheHolesIn) {
    auto polygon = closedSquare(0, 0, 1000, 1000);
    polygon.push_back(closedHole(250, 250, 500, 500));
    const auto bucket = buildBucket(std::move(polygon));

    ASSERT_GT(bucket->vertices.elements(), 0u);
    // Eight distinct corners: x and y add up to 2000 + 1500, and 3500 / 8 is 437.5.
    for (std::size_t i = 0; i < bucket->vertices.elements(); ++i) {
        EXPECT_EQ((std::array<int16_t, 2>{437, 437}), centroidOf(*bucket, i)) << "vertex " << i;
    }
}

// A hole that lies wholly outside the tile is in the buffer: it has no walls.
TEST(FillExtrusionBucket, NoWallsOnAHoleWhollyOutsideTheTile) {
    auto polygon = closedSquare(1000, 1000, 9000, 9000);
    polygon.push_back(closedHole(8300, 3000, 8600, 3300));
    // Only the four walls of the outer ring.
    EXPECT_EQ(4u, wallCount(*buildBucket(std::move(polygon)), 10));

    // The same hole inside the tile has its four walls.
    auto inside = closedSquare(1000, 1000, 9000, 9000);
    inside.push_back(closedHole(3000, 3000, 3300, 3300));
    EXPECT_EQ(8u, wallCount(*buildBucket(std::move(inside)), 10));
}
