#include <gtest/gtest.h>

#include <mbgl/algorithm/contour/intervals.hpp>

using namespace mbgl::algorithm::contour;

// IntervalSchedule mirrors the contour-source style spec's `intervals` field,
// which is a step-by-zoom expression encoded as
//   [output0, stop1, output1, stop2, output2, …, stopN, outputN]
// (always odd length): zoom < stop1 → output0; stop_k ≤ zoom < stop_{k+1} →
// output_k; zoom ≥ stop_N → output_N.

TEST(ContourIntervals, EmptyScheduleReturnsZero) {
    const IntervalSchedule s{};
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 0.0), 0.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 14.0), 0.0);
}

TEST(ContourIntervals, SingleOutputIsConstantForAllZooms) {
    // {200} = "always 200, regardless of zoom". One-element schedule has no
    // stops; it's just output0.
    const IntervalSchedule s{{200.0}};
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 0.0), 200.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 8.5), 200.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 22.0), 200.0);
}

TEST(ContourIntervals, BelowFirstStopReturnsFirstOutput) {
    // {200, 12, 100, 14, 50, 15, 20}:
    //   z<12: 200; 12≤z<14: 100; 14≤z<15: 50; z≥15: 20
    const IntervalSchedule s{{200.0, 12.0, 100.0, 14.0, 50.0, 15.0, 20.0}};
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 0.0), 200.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 11.0), 200.0);
    // Just below the first stop is still the first output.
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 11.99), 200.0);
}

TEST(ContourIntervals, AtAndAboveStopUsesNextOutput) {
    // Step expressions are right-continuous: at zoom == stop, switch to the
    // next output. This matches MapLibre's `step` expression semantics.
    const IntervalSchedule s{{200.0, 12.0, 100.0, 14.0, 50.0, 15.0, 20.0}};
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 12.0), 100.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 13.99), 100.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 14.0), 50.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 14.5), 50.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 15.0), 20.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 22.0), 20.0);
}

TEST(ContourIntervals, FractionalZoomLandsInRightBucket) {
    // Fractional zoom values are common during pinch-zoom. Spot-check a few.
    const IntervalSchedule s{{200.0, 12.0, 100.0, 14.0, 50.0}};
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 12.5), 100.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 13.999), 100.0);
    EXPECT_DOUBLE_EQ(intervalForZoom(s, 14.001), 50.0);
}

TEST(ContourIntervals, IsValidAcceptsWellFormedSchedules) {
    EXPECT_TRUE(isValid(IntervalSchedule{{100.0}}));
    EXPECT_TRUE(isValid(IntervalSchedule{{200.0, 12.0, 100.0}}));
    EXPECT_TRUE(isValid(IntervalSchedule{{200.0, 12.0, 100.0, 14.0, 50.0, 15.0, 20.0}}));
}

TEST(ContourIntervals, IsValidRejectsEmptySchedule) {
    EXPECT_FALSE(isValid(IntervalSchedule{}));
}

TEST(ContourIntervals, IsValidRejectsEvenLength) {
    // Even length means a stop with no output above it, which is malformed.
    EXPECT_FALSE(isValid(IntervalSchedule{{200.0, 12.0}}));
    EXPECT_FALSE(isValid(IntervalSchedule{{200.0, 12.0, 100.0, 14.0}}));
}

TEST(ContourIntervals, IsValidRejectsNonMonotonicStops) {
    // Stops must be strictly increasing.
    EXPECT_FALSE(isValid(IntervalSchedule{{200.0, 14.0, 100.0, 12.0, 50.0}}));
    EXPECT_FALSE(isValid(IntervalSchedule{{200.0, 12.0, 100.0, 12.0, 50.0}}));
}

TEST(ContourIntervals, IsValidRejectsNonPositiveOutputs) {
    EXPECT_FALSE(isValid(IntervalSchedule{{0.0}}));
    EXPECT_FALSE(isValid(IntervalSchedule{{-100.0}}));
    EXPECT_FALSE(isValid(IntervalSchedule{{200.0, 12.0, 0.0}}));
    EXPECT_FALSE(isValid(IntervalSchedule{{200.0, 12.0, -50.0}}));
}
