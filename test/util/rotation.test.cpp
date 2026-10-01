#include <mln/test/util.hpp>

#include <mln/style/rotation.hpp>
#include <mln/style/types.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/interpolate.hpp>

using namespace mln;
using namespace style;

Rotation createRotation(double angle) {
    return Rotation(angle);
}

TEST(Rotation, Calculations) {
    Rotation rot(42);
    EXPECT_EQ(rot, Rotation(42 + 360));
    Rotation rot2(-42);
    EXPECT_EQ(rot2, Rotation(360 - 42));

    mln::util::Interpolator<mln::style::Rotation> i;
    EXPECT_NEAR(i(rot, rot2, 0.5).getAngle(), 0.0, 0.00001);
}
