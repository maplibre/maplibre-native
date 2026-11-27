#include <mln/test/crs.hpp>
#include <mln/test/util.hpp>

#include <mln/map/world_coordinate_helper.hpp>
#include <mln/test/stub_geometry_tile_feature.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/crs.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <new>
#include <span>
#include <stdexcept>

using namespace mln;

namespace {

constexpr double noAnswer = std::numeric_limits<double>::quiet_NaN();

} // namespace

TEST(CRSWorldCoordinateHelper, Identity) {
    const CRSWorldCoordinateHelper& helper = *getRegisteredProjection("identity");
    EXPECT_EQ("identity", helper.name());
    EXPECT_FALSE(helper.wraps());
    EXPECT_EQ(Point<double>(0.75, 0.25), helper.worldFromLatLng({45, 45}));
    EXPECT_EQ(LatLng(45, 45), helper.latLngFromWorld({0.75, 0.25}));
    EXPECT_EQ(180, helper.metersPerWorldUnit({0.5, 0.5}));
}

TEST(CRSWorldCoordinateHelper, LatLngPastTheEdgeOfTheSquareIsUnclamped) {
    const CRSWorldCoordinateHelper& helper = *getRegisteredProjection("identity");
    // The identity has an answer everywhere: a position east of the square keeps its own longitude.
    EXPECT_EQ(LatLng(0, 135), helper.latLngFromWorld({1.25, 0.5}));
    EXPECT_EQ(LatLng(90, 0), helper.latLngFromWorld({0.5, -0.5}));
}

TEST(CRSWorldCoordinateHelper, PositionWithoutAnAnswerUsesTheEdgeTowardTheCenter) {
    const CRSWorldCoordinateHelper helper(std::make_shared<const CRS>(test::disc("test:disc")));
    // The corner of the square has no answer, nor any position past it; the last one with an answer on the line to the
    // center is on the disc.
    const LatLng edge = helper.latLngFromWorld({2, 2});
    EXPECT_NEAR(90, std::hypot(edge.longitude(), edge.latitude()), 1e-6);
    EXPECT_NEAR(-edge.latitude(), edge.longitude(), 1e-9);
    // A position past the square whose clamped one has an answer uses that one.
    EXPECT_EQ(LatLng(0, 90), helper.latLngFromWorld({1.5, 0.5}));
}

TEST(CRSWorldCoordinateHelper, KeepsAHoleInABatchWhileItsNeighborsConvert) {
    int forwardCalls = 0;
    CRS crs = test::disc("test:batch");
    crs.forward = [&forwardCalls, forward = crs.forward](std::span<const std::array<double, 2>> lngLats,
                                                         std::span<std::array<double, 2>> positions) {
        ++forwardCalls;
        forward(lngLats, positions);
    };
    const CRSWorldCoordinateHelper helper(std::make_shared<const CRS>(std::move(crs)));

    const std::array<std::array<double, 2>, 3> lngLats{{{-45, 45}, {noAnswer, noAnswer}, {45, -45}}};
    std::array<std::array<double, 2>, 3> world{};
    helper.worldFromLngLats(lngLats, world);
    EXPECT_EQ(1, forwardCalls);
    EXPECT_EQ((std::array<double, 2>{0.25, 0.25}), world[0]);
    EXPECT_TRUE(std::isnan(world[1][0]) && std::isnan(world[1][1]));
    EXPECT_EQ((std::array<double, 2>{0.75, 0.75}), world[2]);

    // The corner between them has no answer of its own and falls back to the edge of the disc, while they keep theirs.
    const std::array<std::array<double, 2>, 3> places{{{0.25, 0.25}, {1, 1}, {0.75, 0.75}}};
    std::array<std::array<double, 2>, 3> answers{};
    helper.lngLatsFromWorld(places, answers);
    EXPECT_EQ((std::array<double, 2>{-45, 45}), answers[0]);
    EXPECT_NEAR(90, std::hypot(answers[1][0], answers[1][1]), 1e-6);
    EXPECT_NEAR(-answers[1][1], answers[1][0], 1e-9);
    EXPECT_EQ((std::array<double, 2>{45, -45}), answers[2]);
}

TEST(CRSWorldCoordinateHelper, ThrowsWhereTheInverseNeverAnswers) {
    CRS crs = test::disc("test:never");
    crs.inverse = [](std::span<const std::array<double, 2>>, std::span<std::array<double, 2>> lngLats) {
        std::ranges::fill(lngLats, std::array<double, 2>{noAnswer, noAnswer});
    };
    const CRSWorldCoordinateHelper helper(std::make_shared<const CRS>(std::move(crs)));
    EXPECT_THROW(helper.latLngFromWorld({0.5, 0.5}), std::runtime_error);
}

TEST(CRSWorldCoordinateHelper, PassesOnWhatAConverterThrows) {
    CRS crs = test::disc("test:out-of-memory");
    crs.inverse = [](std::span<const std::array<double, 2>>, std::span<std::array<double, 2>>) {
        throw std::bad_alloc();
    };
    const CRSWorldCoordinateHelper helper(std::make_shared<const CRS>(std::move(crs)));
    EXPECT_THROW(helper.latLngFromWorld({0.5, 0.5}), std::bad_alloc);
}

TEST(CRSWorldCoordinateHelper, LatitudeRangeOfAPolarProjection) {
    const CRSWorldCoordinateHelper helper(std::make_shared<const CRS>(test::northPolarStereographic("test:polar")));
    // The opposite pole projects to infinity: the range ends where the meridian stretches ten times as much as at the
    // north pole.
    EXPECT_EQ((std::array<double, 2>{-53, 90}), helper.latitudeRange());
}

TEST(CRSWorldCoordinateHelper, LatitudeRangeOfTheIdentity) {
    const CRSWorldCoordinateHelper& helper = *getRegisteredProjection("identity");
    EXPECT_EQ((std::array<double, 2>{-90, 90}), helper.latitudeRange());
}

TEST(AddProjection, NeedsAColonInTheName) {
    EXPECT_THROW(addProjection(test::disc("disc")), std::invalid_argument);
    EXPECT_FALSE(getRegisteredProjection("disc"));
}

TEST(AddProjection, RejectsATakenName) {
    addProjection(test::disc("test:taken"));
    EXPECT_THROW(addProjection(test::disc("test:taken")), std::invalid_argument);
    removeProjection("test:taken");
}

TEST(AddProjection, NeedsAPositiveExtent) {
    CRS crs = test::disc("test:empty");
    crs.tileMatrixSet.extentAtZoom0 = 0;
    EXPECT_THROW(addProjection(std::move(crs)), std::invalid_argument);
    EXPECT_FALSE(getRegisteredProjection("test:empty"));
}

TEST(AddProjection, NeedsForwardAndInverse) {
    CRS crs = test::disc("test:forward-only");
    crs.inverse = nullptr;
    EXPECT_THROW(addProjection(std::move(crs)), std::invalid_argument);
}

TEST(RemoveProjection, RemovesARegisteredProjection) {
    addProjection(test::disc("test:removed"));
    const auto removed = getRegisteredProjection("test:removed");
    ASSERT_TRUE(removed);
    removeProjection("test:removed");
    EXPECT_FALSE(getRegisteredProjection("test:removed"));

    // As in GL JS, the name is free again, and whatever still holds the removed projection keeps it.
    addProjection(test::wideIdentity("test:removed"));
    EXPECT_EQ(720, getRegisteredProjection("test:removed")->tileMatrixSet().extentAtZoom0);
    EXPECT_EQ(180, removed->tileMatrixSet().extentAtZoom0);
    removeProjection("test:removed");
}

TEST(RemoveProjection, CannotRemoveTheIdentity) {
    EXPECT_THROW(removeProjection("identity"), std::invalid_argument);
    EXPECT_TRUE(getRegisteredProjection("identity"));
}

TEST(ConvertGeometry, ReturnsLatLngThroughTheProjection) {
    const CRSWorldCoordinateHelper& identity = *getRegisteredProjection("identity");
    // The top-left and bottom-right corners of tile 1/1/0 of the identity: lng 0..90, lat 90..0.
    const StubGeometryTileFeature feature(
        {}, FeatureType::LineString, {{{0, 0}, {util::EXTENT, util::EXTENT}}}, PropertyMap{});

    const auto geometry = convertGeometry(feature, CanonicalTileID(1, 1, 0), identity);
    EXPECT_EQ((LineString<double>{{0, 90}, {90, 0}}), geometry.get<LineString<double>>());

    // Mercator's corner of the same tile is at its latitude limit.
    const auto mercator = convertGeometry(feature, CanonicalTileID(1, 1, 0));
    EXPECT_NEAR(85.0511, mercator.get<LineString<double>>()[0].y, 1e-4);
}
