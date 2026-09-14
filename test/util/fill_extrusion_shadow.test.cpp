#include <mln/test/util.hpp>

#include <mln/math/angles.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/projection.hpp>

#include <cmath>

using namespace mln;

namespace {

/// The metres-to-tile-units factor used by FillExtrusionShadowLayerTweaker to convert a building's
/// height into the horizontal shear that flattens it onto the ground plane.
///
/// Kept as a standalone copy of the expression rather than reaching into the tweaker, which needs a
/// live TransformState and PaintParameters. The point of these tests is to pin the *algebra*: it is
/// the one part of the shadow pipeline that is silently wrong rather than visibly broken when it
/// drifts, because a constant error just looks like a different shadow length.
double metersToTileUnits(double latitude, double zoom, uint8_t tileZ) {
    const double metersPerPixel = Projection::getMetersPerPixelAtLatitude(latitude, zoom);
    const double numTiles = static_cast<double>(1ull << tileZ);
    return (1.0 / metersPerPixel) * numTiles * util::EXTENT / Projection::worldSize(std::pow(2.0, zoom));
}

/// The closed form the tweaker's comment claims the expression reduces to.
double metersToTileUnitsClosedForm(double latitude, uint8_t tileZ) {
    const double numTiles = static_cast<double>(1ull << tileZ);
    return numTiles * util::EXTENT / (std::cos(util::deg2rad(latitude)) * util::M2PI * util::EARTH_RADIUS_M);
}

} // namespace

TEST(FillExtrusionShadow, MetersToTileUnitsMatchesClosedForm) {
    // The worldSize terms cancel, so the factor depends only on tile z and latitude -- never on the
    // map's zoom. If this ever stops holding, the shadow will drift while zooming.
    for (uint8_t tileZ : {0, 8, 15, 22}) {
        for (double latitude : {0.0, 37.795, 60.0, -45.0}) {
            for (double zoom : {4.0, 12.0, 17.5}) {
                EXPECT_NEAR(metersToTileUnitsClosedForm(latitude, tileZ),
                            metersToTileUnits(latitude, zoom, tileZ),
                            metersToTileUnitsClosedForm(latitude, tileZ) * 1e-9)
                    << "tileZ=" << int(tileZ) << " lat=" << latitude << " zoom=" << zoom;
            }
        }
    }
}

TEST(FillExtrusionShadow, MetersToTileUnitsIsZoomIndependent) {
    const double reference = metersToTileUnits(0.0, 10.0, 15);
    for (double zoom : {1.0, 5.0, 10.0, 16.0, 22.0}) {
        EXPECT_NEAR(reference, metersToTileUnits(0.0, zoom, 15), reference * 1e-9) << "zoom=" << zoom;
    }
}

TEST(FillExtrusionShadow, MetersToTileUnitsKnownValue) {
    // Hand-checked: a z15 tile spans 40075017/32768 = 1223.1 m across 8192 tile units, so one metre
    // is 8192/1223.1 = 6.698 units. This is the number `u_height_factor` would have got wrong.
    EXPECT_NEAR(6.698, metersToTileUnits(0.0, 15.0, 15), 0.01);

    // `u_height_factor` is -2^z / tileSize / 8, i.e. 8.0 at z15 -- about 19 % too large even at the
    // equator, which is why it must not be reused here.
    const double heightFactor = std::pow(2.0, 15) / util::tileSize_D / 8.0;
    EXPECT_NEAR(8.0, heightFactor, 1e-9);
    EXPECT_GT(heightFactor / metersToTileUnits(0.0, 15.0, 15), 1.19);
}

TEST(FillExtrusionShadow, MetersToTileUnitsScalesWithInverseCosLatitude) {
    // Shadows must get *longer* in tile units towards the poles, because a tile covers less ground
    // there. Dropping this term is the mistake that leaves shadows half-length at latitude 60.
    const double atEquator = metersToTileUnits(0.0, 15.0, 15);
    const double at60 = metersToTileUnits(60.0, 15.0, 15);
    EXPECT_NEAR(2.0, at60 / atEquator, 0.01);

    // Symmetric about the equator.
    EXPECT_NEAR(metersToTileUnits(45.0, 15.0, 15), metersToTileUnits(-45.0, 15.0, 15), 1e-9);
}

TEST(FillExtrusionShadow, MetersToTileUnitsDoublesPerTileZoomLevel) {
    for (uint8_t tileZ = 1; tileZ <= 20; ++tileZ) {
        const double coarser = metersToTileUnits(0.0, 15.0, static_cast<uint8_t>(tileZ - 1));
        EXPECT_NEAR(2.0, metersToTileUnits(0.0, 15.0, tileZ) / coarser, 1e-9) << "tileZ=" << int(tileZ);
    }
}

TEST(FillExtrusionShadow, ShadowDirectionFromAzimuth) {
    // Tile space is +x east, +y south, and the azimuth is the compass bearing the shadow falls
    // towards, so the direction is (sin a, -cos a).
    const auto direction = [](double degrees) {
        const double a = util::deg2rad(degrees);
        return std::pair<double, double>{std::sin(a), -std::cos(a)};
    };

    const auto north = direction(0);
    EXPECT_NEAR(0.0, north.first, 1e-9);
    EXPECT_NEAR(-1.0, north.second, 1e-9); // towards -y, i.e. north

    const auto east = direction(90);
    EXPECT_NEAR(1.0, east.first, 1e-9);
    EXPECT_NEAR(0.0, east.second, 1e-9);

    const auto south = direction(180);
    EXPECT_NEAR(0.0, south.first, 1e-9);
    EXPECT_NEAR(1.0, south.second, 1e-9);

    // The default, 225, must point south-west: -x and +y.
    const auto southWest = direction(225);
    EXPECT_LT(southWest.first, 0.0);
    EXPECT_GT(southWest.second, 0.0);
    EXPECT_NEAR(1.0, std::hypot(southWest.first, southWest.second), 1e-9);
}
