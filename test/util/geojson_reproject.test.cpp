#include <mln/test/crs.hpp>
#include <mln/test/util.hpp>

#include <mln/map/world_coordinate_helper.hpp>
#include <mln/util/geojson_reproject.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

using namespace mln;

namespace {

using Position = mapbox::geometry::point<double>;
using Positions = std::vector<Position>;
using GeoGeometry = mapbox::geometry::geometry<double>;
using GeoFeature = mapbox::feature::feature<double>;
using GeoFeatures = mapbox::feature::feature_collection<double>;

std::shared_ptr<const CRS> polar() {
    return std::make_shared<const CRS>(test::polarStereographic("test:polar"));
}

bool segmentIntersectsWorldSquare(const Position& a, const Position& b) {
    double from = 0;
    double to = 1;
    const std::array<std::array<double, 2>, 4> edges{
        {{-(b.x - a.x), a.x}, {b.x - a.x, 1 - a.x}, {-(b.y - a.y), a.y}, {b.y - a.y, 1 - a.y}}};
    for (const auto& [p, q] : edges) {
        if (p == 0) {
            if (q < 0) {
                return false;
            }
        } else if (p < 0) {
            from = std::max(from, q / p);
        } else {
            to = std::min(to, q / p);
        }
    }
    return from <= to;
}

bool lineIntersectsWorldSquare(const Positions& line) {
    for (size_t i = 1; i < line.size(); ++i) {
        if (segmentIntersectsWorldSquare(line[i - 1], line[i])) {
            return true;
        }
    }
    return false;
}

bool ringContains(const Positions& ring, const Position& point) {
    bool inside = false;
    for (size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++) {
        if ((ring[i].y > point.y) != (ring[j].y > point.y) &&
            point.x < (ring[j].x - ring[i].x) * (point.y - ring[i].y) / (ring[j].y - ring[i].y) + ring[i].x) {
            inside = !inside;
        }
    }
    return inside;
}

Positions ringOf(const GeoJSON& geoJSON) {
    return geoJSON.get<GeoGeometry>().get<mapbox::geometry::polygon<double>>()[0];
}

} // namespace

TEST(ReprojectGeoJSONToWorld, MovesAnIdentityPositionToItsWorldCoordinates) {
    const CRSWorldCoordinateHelper& helper = *getRegisteredProjection("identity");

    const auto result = reprojectGeoJSONToWorld(GeoGeometry{Position{45, 45}}, helper);

    EXPECT_EQ(Position(0.75, 0.25), result.get<GeoGeometry>().get<Position>());
}

TEST(ReprojectGeoJSONToWorld, WalksFeaturesCollectionsAndGeometryCollections) {
    const CRSWorldCoordinateHelper& helper = *getRegisteredProjection("identity");
    const GeoFeatures features{
        GeoFeature{mapbox::geometry::polygon<double>{{{0, 0}, {45, 0}, {45, 45}, {0, 45}, {0, 0}}},
                   PropertyMap{{"name", std::string("a")}},
                   std::string("a")},
        GeoFeature{mapbox::geometry::geometry_collection<double>{
            mapbox::geometry::multi_line_string<double>{{{0, 0}, {45, 45}}}, Position{45, 0}}},
        GeoFeature{mapbox::geometry::empty{}}};

    const auto result = reprojectGeoJSONToWorld(features, helper).get<GeoFeatures>();

    ASSERT_EQ(3u, result.size());
    EXPECT_EQ(Position(0.75, 0.25), result[0].geometry.get<mapbox::geometry::polygon<double>>()[0][2]);
    EXPECT_EQ(std::string("a"), result[0].id.get<std::string>());
    EXPECT_EQ(features[0].properties, result[0].properties);
    const auto& collection = result[1].geometry.get<mapbox::geometry::geometry_collection<double>>();
    EXPECT_EQ(Position(0.75, 0.5), collection[1].get<Position>());
    EXPECT_TRUE(result[2].geometry.is<mapbox::geometry::empty>());
}

TEST(ReprojectGeoJSONToWorld, LeavesOutPositionsWithoutAFinitePlace) {
    const auto halfWorld = std::make_shared<const CRS>(CRS{
        .name = "test:half",
        .forward = test::eachPosition([](const std::array<double, 2>& lngLat) {
            return lngLat[0] > 0 ? std::array<double, 2>{std::numeric_limits<double>::quiet_NaN(), lngLat[1]} : lngLat;
        }),
        .inverse = test::eachPosition(test::unchanged),
        .tileMatrixSet = {.origin = {-90, 90}, .extentAtZoom0 = 180}});
    const CRSWorldCoordinateHelper helper(halfWorld);

    const auto result = reprojectGeoJSONToWorld(
        GeoFeatures{GeoFeature{Position{45, 0}},
                    GeoFeature{mapbox::geometry::multi_point<double>{{-45, 0}, {45, 0}}},
                    GeoFeature{mapbox::geometry::line_string<double>{{-45, 0}, {45, 0}, {-45, 45}}}},
        helper);

    const auto& features = result.get<GeoFeatures>();
    EXPECT_TRUE(features[0].geometry.is<mapbox::geometry::empty>());
    EXPECT_EQ((mapbox::geometry::multi_point<double>{{0.25, 0.5}}),
              features[1].geometry.get<mapbox::geometry::multi_point<double>>());
    EXPECT_EQ((mapbox::geometry::line_string<double>{{0.25, 0.5}, {0.25, 0.25}}),
              features[2].geometry.get<mapbox::geometry::line_string<double>>());
}

TEST(ReprojectGeoJSONToWorld, KeepsAPolygonWithAnEdgeAcrossTheFarPoleOutOfTheWorldSquare) {
    const CRSWorldCoordinateHelper helper(polar());
    const mapbox::geometry::polygon<double> isea4hSouthPoleCell{{{88.3853, -87.67805},
                                                                 {54.96953, -88.57819},
                                                                 {0, -89.52868},
                                                                 {180, -89.31865},
                                                                 {142.1755, -88.10424},
                                                                 {115.34565, -87.43262},
                                                                 {88.3853, -87.67805}}};

    const auto ring = ringOf(reprojectGeoJSONToWorld(GeoGeometry{isea4hSouthPoleCell}, helper));

    EXPECT_FALSE(lineIntersectsWorldSquare(ring));
    EXPECT_FALSE(ringContains(ring, {0.5, 0.5}));
}

TEST(ReprojectGeoJSONToWorld, KeepsALineAlongAParallelPastTheSouthernLimitOutOfTheWorldSquare) {
    const CRSWorldCoordinateHelper helper(polar());

    const auto line = reprojectGeoJSONToWorld(GeoGeometry{mapbox::geometry::line_string<double>{{0, -80}, {180, -80}}},
                                              helper)
                          .get<GeoGeometry>()
                          .get<mapbox::geometry::line_string<double>>();

    EXPECT_FALSE(lineIntersectsWorldSquare(line));
}

TEST(ReprojectGeoJSONToWorld, FillsAnEdgeAlongTheSouthernLimitSoAPolygonOfTheWholeGlobeCoversTheWorldSquare) {
    const CRSWorldCoordinateHelper helper(polar());
    const mapbox::geometry::polygon<double> wholeGlobe{{{-180, -90}, {180, -90}, {180, 90}, {-180, 90}, {-180, -90}}};

    const auto ring = ringOf(reprojectGeoJSONToWorld(GeoGeometry{wholeGlobe}, helper));

    EXPECT_TRUE(ringContains(ring, {0.75, 0.75}));
}

TEST(ReprojectGeoJSONToWorld, KeepsAPolygonThatCoversASmallGridOverAllOfIt) {
    auto crs = test::polarStereographic("test:grid");
    std::array<std::array<double, 2>, 1> center;
    crs.forward(std::array<std::array<double, 2>, 1>{{{-5, 70}}}, center);
    crs.tileMatrixSet = {.origin = {center[0][0] - 500, center[0][1] + 500}, .extentAtZoom0 = 1000};
    const CRSWorldCoordinateHelper helper(std::make_shared<const CRS>(std::move(crs)));
    const mapbox::geometry::polygon<double> rectangle{{{-20, 60}, {10, 60}, {10, 80}, {-20, 80}, {-20, 60}}};

    const auto ring = ringOf(reprojectGeoJSONToWorld(GeoGeometry{rectangle}, helper));

    for (const Position& point : Positions{{0.5, 0.5}, {0.01, 0.01}, {0.99, 0.01}, {0.99, 0.99}, {0.01, 0.99}}) {
        EXPECT_TRUE(ringContains(ring, point)) << point.x << ", " << point.y;
    }
}

TEST(ReprojectFeaturesFromWorld, RoundTripsWithReprojectGeoJSONToWorld) {
    const CRSWorldCoordinateHelper& helper = *getRegisteredProjection("identity");
    const GeoFeatures features{GeoFeature{mapbox::geometry::geometry_collection<double>{
        Position{-30, -30}, mapbox::geometry::multi_polygon<double>{{{{0, 0}, {45, 0}, {45, 45}, {0, 0}}}}}}};

    const auto roundTripped = reprojectFeaturesFromWorld(reprojectGeoJSONToWorld(features, helper).get<GeoFeatures>(),
                                                         helper);

    const auto& geometries = roundTripped[0].geometry.get<mapbox::geometry::geometry_collection<double>>();
    const auto& point = geometries[0].get<Position>();
    EXPECT_NEAR(-30, point.x, 1e-9);
    EXPECT_NEAR(-30, point.y, 1e-9);
    const auto& ring = geometries[1].get<mapbox::geometry::multi_polygon<double>>()[0][0];
    const Positions expected{{0, 0}, {45, 0}, {45, 45}, {0, 0}};
    ASSERT_EQ(expected.size(), ring.size());
    for (size_t i = 0; i < ring.size(); ++i) {
        EXPECT_NEAR(expected[i].x, ring[i].x, 1e-9);
        EXPECT_NEAR(expected[i].y, ring[i].y, 1e-9);
    }
}
