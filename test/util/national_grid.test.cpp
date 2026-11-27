#include <mln/test/national_grid.hpp>

#include <gtest/gtest.h>

#include <array>

using namespace mln;
using namespace mln::test::national_grid;

// The worked examples of Ordnance Survey's "A Guide to Coordinate Systems in Great Britain", V3.6 (2020).

namespace {

double degrees(double wholeDegrees, double minutes, double seconds) {
    return wholeDegrees + (minutes / 60) + (seconds / 3600);
}

constexpr double millimeter = 1e-3;
/// The guide gives angles to 0.0001 arc seconds, so they are good to half of that.
constexpr double guideAngleTolerance = 0.00005 / 3600;

} // namespace

TEST(NationalGrid, ProjectsTheGuidesTransverseMercatorExample) {
    const auto [easting, northing] = nationalGridProjection.forward(degrees(52, 39, 27.2531), degrees(1, 43, 4.5177));

    EXPECT_NEAR(651409.903, easting, millimeter);
    EXPECT_NEAR(313177.270, northing, millimeter);
}

TEST(NationalGrid, UnprojectsTheGuidesTransverseMercatorExample) {
    const auto [latitude, longitude] = nationalGridProjection.inverse(651409.903, 313177.270);
    EXPECT_NEAR(degrees(52, 39, 27.2531), latitude, guideAngleTolerance);
    EXPECT_NEAR(degrees(1, 43, 4.5177), longitude, guideAngleTolerance);

    const auto [easting, northing] = nationalGridProjection.forward(latitude, longitude);
    EXPECT_NEAR(651409.903, easting, millimeter);
    EXPECT_NEAR(313177.270, northing, millimeter);
}

TEST(NationalGrid, TransformsTheGuidesHelmertExample) {
    const std::array<double, 3> etrs89 = cartesianFromLatLngHeight(
        grs80, {.latitude = degrees(53, 36, 43.1653), .longitude = -degrees(1, 39, 51.9920), .height = 299.800});
    EXPECT_NEAR(3790644.900, etrs89[0], millimeter);
    EXPECT_NEAR(-110149.210, etrs89[1], millimeter);
    EXPECT_NEAR(5111482.970, etrs89[2], millimeter);

    const std::array<double, 3> osgb36 = etrs89ToOsgb36(etrs89);
    EXPECT_NEAR(3790269.549, osgb36[0], millimeter);
    EXPECT_NEAR(-110038.064, osgb36[1], millimeter);
    EXPECT_NEAR(5111050.261, osgb36[2], millimeter);

    const LatLngHeight onAiry = latLngHeightFromCartesian(airy1830, osgb36);
    EXPECT_NEAR(degrees(53, 36, 42.2972), onAiry.latitude, guideAngleTolerance);
    EXPECT_NEAR(-degrees(1, 39, 46.5416), onAiry.longitude, guideAngleTolerance);
    EXPECT_NEAR(249.950, onAiry.height, millimeter);

    const auto [easting, northing] = nationalGridProjection.forward(onAiry.latitude, onAiry.longitude);
    EXPECT_NEAR(422297.792, easting, millimeter);
    EXPECT_NEAR(412878.741, northing, millimeter);
}

TEST(NationalGrid, RegisteredProjectionConvertsTheHelmertExampleAtSeaLevel) {
    // The example's point is 299.8 m up, and the registered projection converts at sea level, which moves the
    // point 6 mm.
    constexpr double centimeter = 1e-2;
    const CRS crs = test::nationalGrid("EPSG:27700");
    const std::array<std::array<double, 2>, 1> lngLat{{{-degrees(1, 39, 51.9920), degrees(53, 36, 43.1653)}}};
    std::array<std::array<double, 2>, 1> position{};
    crs.forward(lngLat, position);
    EXPECT_NEAR(422297.792, position[0][0], centimeter);
    EXPECT_NEAR(412878.741, position[0][1], centimeter);

    std::array<std::array<double, 2>, 1> back{};
    crs.inverse(position, back);
    std::array<std::array<double, 2>, 1> again{};
    crs.forward(back, again);
    EXPECT_NEAR(position[0][0], again[0][0], centimeter);
    EXPECT_NEAR(position[0][1], again[0][1], centimeter);
}
