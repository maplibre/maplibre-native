#include <mln/test/util.hpp>

#include <mln/renderer/fade_zoom_table.hpp>

using namespace mln;

namespace {
constexpr Duration fade = Milliseconds(300);
const TimePoint start = TimePoint(Seconds(100));
} // namespace

TEST(FadeZoomTable, FirstUpdateTakesTargets) {
    FadeZoomTable table;
    EXPECT_FALSE(table.update(7.5, start, fade));
    EXPECT_EQ(1.0f, table.opacity(0.0f));
    EXPECT_EQ(1.0f, table.opacity(7.0f));
    EXPECT_EQ(1.0f, table.opacity(7.5f));
    EXPECT_EQ(0.0f, table.opacity(7.5625f));
    EXPECT_EQ(0.0f, table.opacity(8.0f));
    EXPECT_FALSE(table.isFading());
}

TEST(FadeZoomTable, FadesInOverTheDuration) {
    FadeZoomTable table;
    table.update(6.9, start, fade);
    EXPECT_EQ(0.0f, table.opacity(7.0f));

    // Crossing zoom 7 starts the fade; no time has passed yet.
    EXPECT_TRUE(table.update(7.1, start, fade));
    EXPECT_EQ(0.0f, table.opacity(7.0f));

    EXPECT_TRUE(table.update(7.1, start + Milliseconds(150), fade));
    EXPECT_FLOAT_EQ(0.5f, table.opacity(7.0f));
    // Entries not crossed are unchanged.
    EXPECT_EQ(1.0f, table.opacity(6.0f));
    EXPECT_EQ(0.0f, table.opacity(8.0f));

    EXPECT_FALSE(table.update(7.1, start + Milliseconds(300), fade));
    EXPECT_EQ(1.0f, table.opacity(7.0f));
    EXPECT_FALSE(table.isFading());
}

TEST(FadeZoomTable, FadesOutWhenZoomingOut) {
    FadeZoomTable table;
    table.update(9.0, start, fade);
    EXPECT_EQ(1.0f, table.opacity(9.0f));

    EXPECT_TRUE(table.update(8.9, start, fade));
    EXPECT_TRUE(table.update(8.9, start + Milliseconds(75), fade));
    EXPECT_FLOAT_EQ(0.75f, table.opacity(9.0f));
    EXPECT_FALSE(table.update(8.9, start + Milliseconds(400), fade));
    EXPECT_EQ(0.0f, table.opacity(9.0f));
}

TEST(FadeZoomTable, ReversesFromWhereItIs) {
    FadeZoomTable table;
    table.update(4.9, start, fade);
    table.update(5.2, start, fade);
    table.update(5.2, start + Milliseconds(150), fade);
    EXPECT_FLOAT_EQ(0.5f, table.opacity(5.0f));

    // Zooming back out half-way through the fade takes the remaining half to disappear.
    EXPECT_TRUE(table.update(4.9, start + Milliseconds(225), fade));
    EXPECT_FLOAT_EQ(0.25f, table.opacity(5.0f));
    EXPECT_FALSE(table.update(4.9, start + Milliseconds(300), fade));
    EXPECT_EQ(0.0f, table.opacity(5.0f));
}

TEST(FadeZoomTable, ZeroDurationDoesNotFade) {
    FadeZoomTable table;
    table.update(3.0, start, fade);
    EXPECT_FALSE(table.update(12.0, start + Milliseconds(16), Duration::zero()));
    EXPECT_EQ(1.0f, table.opacity(12.0f));
    EXPECT_FALSE(table.update(3.0, start + Milliseconds(32), Duration::zero()));
    EXPECT_EQ(0.0f, table.opacity(12.0f));
}

TEST(FadeZoomTable, WholeZoomIsDrawnEvenWhenSlightlyBelow) {
    FadeZoomTable table;
    table.update(10.0 - 1e-9, start, fade);
    EXPECT_EQ(1.0f, table.opacity(10.0f));
    EXPECT_EQ(0.0f, table.opacity(10.0625f));
}

TEST(FadeZoomTable, RoundsToSixteenthsAndClamps) {
    EXPECT_EQ(FadeZoomTable::index(7.0f), FadeZoomTable::index(7.03f));
    EXPECT_EQ(FadeZoomTable::index(7.0f) + 1, FadeZoomTable::index(7.04f));
    EXPECT_EQ(112u, FadeZoomTable::index(7.0f));
    // Half a step, exactly representable: rounds up, as in the shaders.
    EXPECT_EQ(161u, FadeZoomTable::index(10.03125f));
    EXPECT_EQ(0u, FadeZoomTable::index(-3.0f));
    EXPECT_EQ(shaders::fadeZoomTableSize - 1, FadeZoomTable::index(40.0f));
}

TEST(FadeZoomTable, VersionChangesOnlyWithValues) {
    FadeZoomTable table;
    table.update(5.0, start, fade);
    const auto version = table.getVersion();
    table.update(5.0, start + Milliseconds(16), fade);
    table.update(5.05, start + Milliseconds(32), fade);
    EXPECT_EQ(version, table.getVersion());

    // The fade starts at the crossing and changes the values from the next update on.
    table.update(5.1, start + Milliseconds(48), fade);
    EXPECT_EQ(version, table.getVersion());
    table.update(5.1, start + Milliseconds(64), fade);
    EXPECT_NE(version, table.getVersion());
}

TEST(FadeZoomTable, IdleTimeDoesNotCount) {
    FadeZoomTable table;
    table.update(3.0, start, fade);

    // Nothing rendered for ten seconds, then the zoom crosses 3.0625: the fade starts now.
    EXPECT_TRUE(table.update(3.1, start + Seconds(10), fade));
    EXPECT_EQ(0.0f, table.opacity(3.0625f));
    EXPECT_TRUE(table.update(3.1, start + Seconds(10) + Milliseconds(150), fade));
    EXPECT_FLOAT_EQ(0.5f, table.opacity(3.0625f));
}
