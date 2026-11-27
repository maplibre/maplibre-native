#include <mln/test/crs.hpp>
#include <mln/util/tile_range.hpp>
#include <mln/util/geo.hpp>
#include <mln/map/transform.hpp>
#include <mln/map/world_coordinate_helper.hpp>

#include <cmath>
#include <memory>

#include <gtest/gtest.h>

using namespace mln;

TEST(TileRange, ContainsWorld) {
    auto range = util::TileRange::fromLatLngBounds(LatLngBounds::world(), 0);
    EXPECT_TRUE(range.contains(CanonicalTileID(0, 0, 0)));
    EXPECT_FALSE(range.contains(CanonicalTileID(10, 0, 0)));
}

TEST(TileRange, ContainsBoundsFromTile) {
    {
        const LatLngBounds bounds{CanonicalTileID(3, 7, 0)};
        auto range = util::TileRange::fromLatLngBounds(bounds, 3);
        EXPECT_TRUE(range.contains(CanonicalTileID(3, 7, 0)));
    }
    {
        const LatLngBounds bounds{CanonicalTileID(10, 162, 395)};
        auto range = util::TileRange::fromLatLngBounds(bounds, 10);
        EXPECT_TRUE(range.contains(CanonicalTileID(10, 162, 395)));
    }
}

TEST(TileRange, ContainsMultiZoom) {
    auto wrappedBounds = LatLngBounds::hull({37.6609, -122.5744}, {37.8271, -122.3204});
    auto range = util::TileRange::fromLatLngBounds(wrappedBounds, 5, 13);
    EXPECT_FALSE(range.contains(CanonicalTileID(0, 0, 0)));
    EXPECT_FALSE(range.contains(CanonicalTileID(5, 3, 11)));
    EXPECT_FALSE(range.contains(CanonicalTileID(6, 9, 22)));
    EXPECT_TRUE(range.contains(CanonicalTileID(5, 5, 12)));
    EXPECT_TRUE(range.contains(CanonicalTileID(6, 10, 24)));
    EXPECT_TRUE(range.contains(CanonicalTileID(13, 1310, 3166)));
}

TEST(TileRange, ContainsIntersectingTiles) {
    auto bounds = LatLngBounds::hull({37.6609, -122.5744}, {37.8271, -122.3204});
    auto range = util::TileRange::fromLatLngBounds(bounds, 13);
    EXPECT_FALSE(range.contains(CanonicalTileID(13, 1316, 3100)));
    EXPECT_TRUE(range.contains(CanonicalTileID(13, 1310, 3166)));
}

TEST(TileRange, ContainsWrappedBounds) {
    auto wrappedBounds = LatLngBounds::hull({37.6609, 237.6796}, {37.8271, 237.4256});
    auto range = util::TileRange::fromLatLngBounds(wrappedBounds, 13);
    EXPECT_FALSE(range.contains(CanonicalTileID(13, 1316, 3100)));
    EXPECT_TRUE(range.contains(CanonicalTileID(13, 1310, 3166)));
}

TEST(TileRange, ContainsBoundsCrossingAntimeridian) {
    {
        auto crossingBounds = LatLngBounds::hull({-20.9615, -214.309}, {19.477, -155.830});
        auto range = util::TileRange::fromLatLngBounds(crossingBounds, 1);
        EXPECT_TRUE(range.contains(CanonicalTileID(1, 1, 1)));
        EXPECT_TRUE(range.contains(CanonicalTileID(1, 0, 0)));
    }
    {
        auto crossingBounds = LatLngBounds::hull({-20.9615, -214.309}, {19.477, -155.830});
        auto range = util::TileRange::fromLatLngBounds(crossingBounds, 6);
        EXPECT_FALSE(range.contains(CanonicalTileID(6, 55, 34)));
        EXPECT_FALSE(range.contains(CanonicalTileID(6, 5, 28)));
        EXPECT_TRUE(range.contains(CanonicalTileID(6, 63, 28)));
    }
}

TEST(TileRange, FromWorldBox) {
    // Lng 0..90 and lat 45..90 of the identity: the top row of the north-east quarter.
    auto range = util::TileRange::fromWorldBox({0.5, 0, 1, 0.25}, 0, 2);
    EXPECT_TRUE(range.contains(CanonicalTileID(2, 2, 0)));
    EXPECT_TRUE(range.contains(CanonicalTileID(2, 3, 0)));
    EXPECT_FALSE(range.contains(CanonicalTileID(2, 2, 1)));
    EXPECT_FALSE(range.contains(CanonicalTileID(2, 1, 0)));
    EXPECT_TRUE(range.contains(CanonicalTileID(1, 1, 0)));
    EXPECT_FALSE(range.contains(CanonicalTileID(1, 0, 0)));
    EXPECT_TRUE(range.contains(CanonicalTileID(0, 0, 0)));
}

TEST(TileRange, FromWorldBoxOutsideTheSquare) {
    auto range = util::TileRange::fromWorldBox({1.5, 0, 2, 1}, 0, 2);
    EXPECT_FALSE(range.contains(CanonicalTileID(0, 0, 0)));
    EXPECT_FALSE(range.contains(CanonicalTileID(2, 3, 0)));
}

namespace {

const CRSWorldCoordinateHelper& polarStereographic() {
    static const CRSWorldCoordinateHelper helper(std::make_shared<const CRS>(test::polarStereographic("test:polar")));
    return helper;
}

} // namespace

TEST(TileRange, FromTheWorldBoxOfThePolarCapNorthOf60) {
    const auto range = util::TileRange::fromWorldBox(
        latLngBoxToWorldBox(polarStereographic(), LatLngBounds::hull({60, -180}, {90, 180})), 2, 2);
    for (uint32_t x = 0; x < 4; ++x) {
        for (uint32_t y = 0; y < 4; ++y) {
            EXPECT_TRUE(range.contains(CanonicalTileID(2, x, y))) << x << "," << y;
        }
    }
}

TEST(TileRange, FromTheWorldBoxOfAPolarCapThatNoSampleReachesTheEdgeOf) {
    // 45 E is the rightmost point of the 60 N circle, between two of the box's longitude samples.
    const auto range = util::TileRange::fromWorldBox(
        latLngBoxToWorldBox(polarStereographic(), LatLngBounds::hull({60, -170}, {90, 170})), 13, 13);
    const auto world = polarStereographic().worldFromLatLng({60, 45});
    const double tiles = std::pow(2.0, 13);
    EXPECT_TRUE(range.contains(CanonicalTileID(
        13, static_cast<uint32_t>(std::floor(world.x * tiles)), static_cast<uint32_t>(std::floor(world.y * tiles)))));
}
