#include <mln/test/util.hpp>

#include <mln/text/overlap_mode.hpp>

using namespace mln;

TEST(OverlapMode, FallsBackToAllowOverlapWhenUnset) {
    EXPECT_EQ(OverlapMode::Never, getOverlapMode(style::IconOverlapType::Undefined, false));
    EXPECT_EQ(OverlapMode::Always, getOverlapMode(style::IconOverlapType::Undefined, true));
}

TEST(OverlapMode, ExplicitValuesWin) {
    EXPECT_EQ(OverlapMode::Never, getOverlapMode(style::IconOverlapType::Never, true));
    EXPECT_EQ(OverlapMode::Always, getOverlapMode(style::IconOverlapType::Always, false));
    EXPECT_EQ(OverlapMode::Cooperative, getOverlapMode(style::IconOverlapType::Cooperative, false));
}
