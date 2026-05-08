#include <gtest/gtest.h>

#include <mbgl/algorithm/contour/smoothing.hpp>

#include <array>
#include <cmath>
#include <vector>

using namespace mbgl::algorithm::contour;

// Smoothing pipeline applied to raw marching-squares output before MVT
// emission. Two stages:
// Douglas-Peucker simplification first to drop sub-pixel wiggle, then Chaikin
// corner-cutting to round off the remaining corners.

namespace {

constexpr double kDoubleEps = 1e-9;

bool near(const Point2D& a, const Point2D& b, double eps = 1e-6) {
    return std::abs(a[0] - b[0]) < eps && std::abs(a[1] - b[1]) < eps;
}

} // namespace

// ---- Douglas-Peucker -----------------------------------------------------

TEST(ContourSmoothingDP, KeepsEndpointsForTrivialInput) {
    const std::vector<Point2D> pts{{0, 0}, {10, 0}};
    const auto out = douglasPeucker(pts, 1.0);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_TRUE(near(out[0], {0, 0}));
    EXPECT_TRUE(near(out[1], {10, 0}));
}

TEST(ContourSmoothingDP, EmptyAndSinglePointPassedThrough) {
    EXPECT_TRUE(douglasPeucker(std::vector<Point2D>{}, 1.0).empty());
    {
        const std::vector<Point2D> one{{5, 5}};
        const auto out = douglasPeucker(one, 1.0);
        ASSERT_EQ(out.size(), 1u);
        EXPECT_TRUE(near(out[0], {5, 5}));
    }
}

TEST(ContourSmoothingDP, DropsCollinearMidpoints) {
    // 5 perfectly collinear points along y=0; everything between endpoints
    // sits at perpendicular distance 0 → all dropped.
    const std::vector<Point2D> pts{{0, 0}, {2, 0}, {5, 0}, {8, 0}, {10, 0}};
    const auto out = douglasPeucker(pts, 0.5);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_TRUE(near(out[0], {0, 0}));
    EXPECT_TRUE(near(out[1], {10, 0}));
}

TEST(ContourSmoothingDP, KeepsOutOfToleranceMidpoint) {
    // Triangular bend — mid point sits at perpendicular distance 5 from the
    // chord (0,0)-(10,0). Tolerance 1.0 → mid point stays.
    const std::vector<Point2D> pts{{0, 0}, {5, 5}, {10, 0}};
    const auto out = douglasPeucker(pts, 1.0);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_TRUE(near(out[1], {5, 5}));
}

TEST(ContourSmoothingDP, AcceptsExactlyAtThresholdPoint) {
    // Mid point at perpendicular distance 1.0 from the chord, tolerance 1.0.
    // The classic DP rule is strict-greater (`d > eps`) so a point exactly at
    // the threshold gets dropped — verify we honour that convention (matches
    // the Go reference).
    const std::vector<Point2D> pts{{0, 0}, {5, 1}, {10, 0}};
    const auto out = douglasPeucker(pts, 1.0);
    EXPECT_EQ(out.size(), 2u);
}

TEST(ContourSmoothingDP, ZeroToleranceKeepsAllNonCollinearPoints) {
    const std::vector<Point2D> pts{{0, 0}, {3, 1}, {6, 1}, {10, 0}};
    const auto out = douglasPeucker(pts, 0.0);
    EXPECT_EQ(out.size(), 4u);
}

TEST(ContourSmoothingDP, HandlesDuplicateEndpointsLine) {
    // First == last is a degenerate "line" — perpDist falls back to point
    // distance. Point at distance > eps should survive.
    const std::vector<Point2D> pts{{0, 0}, {3, 4}, {0, 0}};
    const auto out = douglasPeucker(pts, 1.0);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_TRUE(near(out[1], {3, 4}));
}

// ---- Chaikin -------------------------------------------------------------

TEST(ContourSmoothingChaikin, ZeroIterationsReturnsInput) {
    const std::vector<Point2D> pts{{0, 0}, {10, 10}, {20, 0}};
    const auto out = chaikin(pts, 0);
    ASSERT_EQ(out.size(), pts.size());
    for (std::size_t i = 0; i < pts.size(); i++) EXPECT_TRUE(near(out[i], pts[i]));
}

TEST(ContourSmoothingChaikin, ShortPolylinesPassThrough) {
    EXPECT_TRUE(chaikin(std::vector<Point2D>{}, 2).empty());
    EXPECT_EQ(chaikin(std::vector<Point2D>{{0, 0}}, 2).size(), 1u);
    EXPECT_EQ(chaikin(std::vector<Point2D>{{0, 0}, {1, 1}}, 2).size(), 2u);
}

TEST(ContourSmoothingChaikin, OneIterationPinsEndpointsAndCutsInteriorCorners) {
    // Three-point bend with a single interior corner. Chaikin pins the
    // endpoints exactly and emits ONE inner point per end-segment (the q
    // for the first segment, the s for the last) to avoid coincident-with-
    // pinned-point degeneracy. Total output: pinned start + 2 inner + pinned
    // end = 4 points (n=3 → 2n-2 = 4).
    const std::vector<Point2D> pts{{0, 0}, {10, 10}, {20, 0}};
    const auto out = chaikin(pts, 1);
    ASSERT_EQ(out.size(), 4u);
    EXPECT_TRUE(near(out[0], {0, 0})) << "start pinned";
    EXPECT_TRUE(near(out[3], {20, 0})) << "end pinned";
    // First segment q=0.75*(0,0)+0.25*(10,10)=(2.5,2.5) skipped (next to
    // pinned start). s=0.25*(0,0)+0.75*(10,10)=(7.5,7.5) emitted.
    EXPECT_TRUE(near(out[1], {7.5, 7.5}));
    // Last segment q=0.75*(10,10)+0.25*(20,0)=(12.5,7.5) emitted. s=0.25*
    // (10,10)+0.75*(20,0)=(17.5,2.5) skipped (next to pinned end).
    EXPECT_TRUE(near(out[2], {12.5, 7.5}));
}

TEST(ContourSmoothingChaikin, TwoIterationsCompoundCornerCutting) {
    // Sanity: 2 iterations on a 3-point bend produces 6 points (4 → 2*4-2).
    // Endpoints still pinned, monotonic-x ordering preserved, all interior
    // points lie within the original bounding box.
    const std::vector<Point2D> pts{{0, 0}, {10, 10}, {20, 0}};
    const auto out = chaikin(pts, 2);
    ASSERT_EQ(out.size(), 6u);
    EXPECT_TRUE(near(out.front(), {0, 0}));
    EXPECT_TRUE(near(out.back(), {20, 0}));
    for (std::size_t i = 1; i < out.size(); i++) {
        EXPECT_GE(out[i][0], out[i - 1][0] - kDoubleEps) << "x monotonic";
    }
    for (const auto& p : out) {
        EXPECT_GE(p[0], 0.0);
        EXPECT_LE(p[0], 20.0);
        EXPECT_GE(p[1], 0.0);
        EXPECT_LE(p[1], 10.0 + kDoubleEps);
    }
}

TEST(ContourSmoothingChaikin, NegativeIterationsTreatedAsZero) {
    const std::vector<Point2D> pts{{0, 0}, {10, 10}, {20, 0}};
    const auto out = chaikin(pts, -3);
    EXPECT_EQ(out.size(), pts.size());
}
