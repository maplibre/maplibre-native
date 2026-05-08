#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include <mbgl/algorithm/contour/height_tile.hpp>

using namespace mbgl::algorithm::contour;

// HeightTile is a thin row-major view over a height grid. It centralises the
// "sample with NaN-on-OOB" helper used by the marching-squares cell loop, and
// gives Task 6 a place to hang overzoom / upsample logic. Pure C++; no
// MapLibre deps.

TEST(ContourHeightTile, SampleInBoundsReturnsValue) {
    const std::vector<std::int16_t> heights{
        10, 20, 30,
        40, 50, 60,
    };
    const HeightTile t{heights, 3, 2};
    EXPECT_DOUBLE_EQ(sample(t, 0, 0), 10.0);
    EXPECT_DOUBLE_EQ(sample(t, 1, 0), 20.0);
    EXPECT_DOUBLE_EQ(sample(t, 2, 0), 30.0);
    EXPECT_DOUBLE_EQ(sample(t, 0, 1), 40.0);
    EXPECT_DOUBLE_EQ(sample(t, 2, 1), 60.0);
}

TEST(ContourHeightTile, SampleOutOfBoundsReturnsNaN) {
    const std::vector<std::int16_t> heights{
        1, 2,
        3, 4,
    };
    const HeightTile t{heights, 2, 2};
    EXPECT_TRUE(std::isnan(sample(t, -1, 0)));
    EXPECT_TRUE(std::isnan(sample(t, 0, -1)));
    EXPECT_TRUE(std::isnan(sample(t, 2, 0)));
    EXPECT_TRUE(std::isnan(sample(t, 0, 2)));
    EXPECT_TRUE(std::isnan(sample(t, 100, 100)));
    EXPECT_TRUE(std::isnan(sample(t, -100, -100)));
}

TEST(ContourHeightTile, EmptyTileReturnsNaNForAnyCoord) {
    const HeightTile t{{}, 0, 0};
    EXPECT_TRUE(std::isnan(sample(t, 0, 0)));
    EXPECT_TRUE(std::isnan(sample(t, 1, 1)));
}

TEST(ContourHeightTile, NegativeSamplesPreserved) {
    // DEM data can be below sea level (Death Valley, Dead Sea, etc.).
    const std::vector<std::int16_t> heights{-86, 0, 100, 8848};
    const HeightTile t{heights, 2, 2};
    EXPECT_DOUBLE_EQ(sample(t, 0, 0), -86.0);
    EXPECT_DOUBLE_EQ(sample(t, 1, 0), 0.0);
    EXPECT_DOUBLE_EQ(sample(t, 0, 1), 100.0);
    EXPECT_DOUBLE_EQ(sample(t, 1, 1), 8848.0);
}

TEST(ContourHeightTile, RowMajorOrderingMatchesIsolinesConvention) {
    // The isolines algorithm indexes samples as `heights[y * width + x]`.
    // Verify HeightTile follows the same convention so callers can swap to it
    // without re-deriving indices.
    //
    //   y=0:  100 200 300
    //   y=1:  400 500 600
    //
    // sample(t, 1, 1) should return the (x=1, y=1) value = 500.
    const std::vector<std::int16_t> heights{
        100, 200, 300,
        400, 500, 600,
    };
    const HeightTile t{heights, 3, 2};
    EXPECT_DOUBLE_EQ(sample(t, 1, 1), 500.0);
}
