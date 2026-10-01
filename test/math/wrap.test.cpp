#include <mln/test/util.hpp>

#include <mln/util/constants.hpp>
#include <mln/math/wrap.hpp>

using namespace mln;

TEST(Math, WrapHigherValue) {
    ASSERT_DOUBLE_EQ(4.0, util::wrap(16.0, 0.0, 12.0));
}

TEST(Math, WrapLowerValue) {
    ASSERT_DOUBLE_EQ(9.0, util::wrap(-3.0, 0.0, 12.0));
}

TEST(Math, WrapInRangeValue) {
    ASSERT_DOUBLE_EQ(4.0, util::wrap(4.0, 0.0, 12.0));
}

TEST(Math, WrapMaxValue) {
    ASSERT_DOUBLE_EQ(0.0, util::wrap(12.0, 0.0, 12.0));
}

TEST(Math, WrapMinValue) {
    ASSERT_DOUBLE_EQ(0.0, util::wrap(0.0, 0.0, 12.0));
}
