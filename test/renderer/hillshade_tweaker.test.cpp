#include <mln/test/util.hpp>

#include <mln/map/world_coordinate_helper.hpp>
#include <mln/renderer/layers/hillshade_layer_tweaker.hpp>
#include <mln/renderer/layers/hillshade_prepare_layer_tweaker.hpp>
#include <mln/tile/tile_id.hpp>

#include <cmath>

using namespace mln;

TEST(HillshadeTweaker, WorldWidthOfTheProjection) {
    // Mercator's, with the prepare shader's 3, is the 28.2562 the shader always used.
    EXPECT_EQ(25.2562f, hillshadeLog2WorldWidth(*mercatorWorldCoordinateHelper()));
    // The identity's world is 180 units wide, its units counting as meters.
    const CRSWorldCoordinateHelper& identity = *getRegisteredProjection("identity");
    EXPECT_FLOAT_EQ(std::log2(180.0f), hillshadeLog2WorldWidth(identity));
}

TEST(HillshadeTweaker, NoLatitudeRangeInAWorldThatDoesNotWrap) {
    const UnwrappedTileID tileID(12, 758, 100);
    const CRSWorldCoordinateHelper& identity = *getRegisteredProjection("identity");
    EXPECT_EQ((std::array<float, 2>{0, 0}), hillshadeLatRange(tileID, identity));

    // The same tile row in Mercator is past 84N, where the shader divides slopes by cos(latitude).
    const auto mercator = hillshadeLatRange(tileID, *mercatorWorldCoordinateHelper());
    EXPECT_GT(mercator[1], 84);
    EXPECT_GT(mercator[0], mercator[1]);
}
