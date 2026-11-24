#include <mln/test/util.hpp>

#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/shaders/layer_ubo.hpp>
#include <mln/util/constants.hpp>

#include <mln/test/stub_geometry_tile_feature.hpp>

#include <array>
#include <cstdint>

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
