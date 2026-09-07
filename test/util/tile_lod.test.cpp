#include <mln/util/tile_lod.hpp>
#include <mln/math/angles.hpp>

#include <cmath>
#include <gtest/gtest.h>

using namespace mln;

namespace {

// TransformState's default field of view, in degrees.
constexpr double fov = 36.869897645844020;

} // namespace

// Oracle: GL JS `createCalculateTileZoomFunction(9.314, 3.0)` on the same inputs.
TEST(TileLod, TileZoomMatchesGLJS) {
    const util::TileZoomFunction tileZoom(fov);
    // Straight down: the center tile loads at the requested zoom plus the field-of-view term.
    EXPECT_NEAR(tileZoom(10, 0, 1, 1), 10.076001546722525, 1e-9);
    // A tile two camera heights away loads coarser.
    EXPECT_NEAR(tileZoom(10, 2, 1, 1), 8.3347577214993152, 1e-9);
    // Pitched 60 degrees: the center tile, a nearer one and a farther one.
    EXPECT_NEAR(tileZoom(12, std::sqrt(3.0) / 2, 0.5, 1), 11.576175751895164, 1e-9);
    EXPECT_NEAR(tileZoom(12, 0.2, 0.5, 1), 12.915426593532230, 1e-9);
    EXPECT_NEAR(tileZoom(12, 4, 0.5, 1), 8.5597502507634662, 1e-9);
    // Toward the horizon.
    EXPECT_NEAR(tileZoom(14, 20, std::cos(util::deg2rad(80.0)), 1), 6.3313403402822646, 1e-9);
}

// Oracle: GL JS `createCalculateTileZoomFunction(1.0, 10.0)` and `(10.0, 1.0)`, the pairs its own tests use.
TEST(TileLod, TileZoomParameters) {
    EXPECT_NEAR(util::TileZoomFunction(fov, 1.0, 10.0)(12, 4, 0.5, 1), 12.662254753200273, 1e-9);
    EXPECT_NEAR(util::TileZoomFunction(fov, 10.0, 1.0)(12, 4, 0.5, 1), 8.1871079138584975, 1e-9);
}

// Oracle: GL JS `getElevationForTileCulling` on the same inputs.
TEST(TileLod, ElevationForTileCulling) {
    EXPECT_DOUBLE_EQ(util::elevationForTileCulling(0, fov), 0.0);
    EXPECT_NEAR(util::elevationForTileCulling(60, fov), 139.49829409740033, 1e-9);
    EXPECT_NEAR(util::elevationForTileCulling(70, fov), 472.83162743073365, 1e-9);
    EXPECT_DOUBLE_EQ(util::elevationForTileCulling(85, fov), 500.0);
    // Terrain lifts the allowance by the center elevation; taller content raises the assumed feature height.
    EXPECT_NEAR(util::elevationForTileCulling(70, fov, 1200, 800), 1956.5306038891738, 1e-9);
    EXPECT_NEAR(util::elevationForTileCulling(70, fov, 0, 900), 851.09692937532054, 1e-9);
}
