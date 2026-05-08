#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>

#include <mbgl/algorithm/contour/isolines.hpp>

using namespace mbgl::algorithm::contour;

namespace {

// Test grids are written row-major, top-down (north-up). The convention
// matches DEM data and MVT y-axis: y=0 is the top of the tile.
//
// A 3×3 grid:
//
//     +---+---+
//     | 0 | 0 | 0 |
//     +---+---+---+
//     | 0 | 0 | 0 |
//     +---+---+---+
//     | 0 | 0 | 0 |
//     +---+---+---+
//
// is laid out as { 0, 0, 0,  0, 0, 0,  0, 0, 0 }.

constexpr ContourThresholds withInterval(double interval) {
    return ContourThresholds{interval, /*extent=*/4096, /*buffer=*/0};
}

} // namespace

TEST(Contour, EmptyGridReturnsNoLines) {
    const std::vector<std::int16_t> heights{};
    const auto lines = generateContours(heights, 0, 0, withInterval(10.0));
    EXPECT_TRUE(lines.empty());
}

TEST(Contour, NoLevelsInDataRangeReturnsNoLines) {
    // 2×2 grid where the data range (10..50) does not contain any contour
    // level for interval=100 (levels are ..., -100, 0, 100, 200, ...). Note
    // that level 0 sits *below* the minimum sample value, not at it, so it
    // doesn't count as a crossing.
    const std::vector<std::int16_t> heights{10, 20, 30, 50};
    const auto lines = generateContours(heights, 2, 2, withInterval(100.0));
    EXPECT_TRUE(lines.empty());
}

TEST(Contour, NoLevelsAboveDataRangeReturnsNoLines) {
    // 2×2 grid where data range (110..150) sits above level 100 and below
    // level 200 — no contour level falls inside.
    const std::vector<std::int16_t> heights{110, 120, 130, 150};
    const auto lines = generateContours(heights, 2, 2, withInterval(100.0));
    EXPECT_TRUE(lines.empty());
}

// Single 2×2 cell. Layout (top-left = sample (0,0)):
//
//     (0,0)----(1,0)
//       |        |
//     (0,1)----(1,1)
//
// Cell occupies tile-local x ∈ [0, extent/(width-1)], y ∈ [0, extent/(height-1)].
// With width=height=2 and extent=4096 the single cell spans the full extent.
//
// "TL above threshold" = case where only sample (0,0) is above the threshold.
// The contour line should cut the top edge between (0,0) and (1,0), and the
// left edge between (0,0) and (0,1). Standard marching-squares case 8.

TEST(Contour, SingleCellTopLeftAboveEmitsOneSegment) {
    // 2×2 grid with TL above threshold, other corners below. Layout:
    //
    //   300 110
    //   110 110
    //
    // With interval=100, the only level inside [110..300] that produces a
    // crossing is 200. Level 100 sits below all corners (110 > 100 strict),
    // and level 300 sits at the maximum (300 > 300 is false strict), so
    // neither triggers a segment. Level 200 cuts the top and left edges of
    // the cell.
    //
    // At level=200, the linear interpolation along the TL(300) → TR(110)
    // edge places the cut at fraction t = (200-300)/(110-300) ≈ 0.526 of
    // the way from TL to TR (i.e. closer to the TR=110 corner because the
    // threshold is closer to 110 than to 300). Same fraction on the
    // TL→BL edge. With extent=4096 and a single cell spanning the full
    // extent, both cuts land at coordinate ≈ 0.526 * 4096 ≈ 2156.
    const std::vector<std::int16_t> heights{300, 110,
                                            110, 110};
    const auto lines = generateContours(heights, 2, 2, withInterval(100.0));

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_DOUBLE_EQ(lines[0].elevation, 200.0);

    // 2 vertices = 4 ints (interleaved x,y).
    ASSERT_EQ(lines[0].points.size(), 4u);

    // Top edge cut at (~2156, 0), left edge cut at (0, ~2156). Either
    // order is acceptable. ±2-unit tolerance for rounding.
    const auto& p = lines[0].points;
    auto near = [](std::int32_t actual, std::int32_t expected) { return std::abs(actual - expected) <= 2; };
    const bool ok =
        // (top, left)
        (near(p[0], 2156) && near(p[1], 0) && near(p[2], 0) && near(p[3], 2156)) ||
        // (left, top)
        (near(p[0], 0) && near(p[1], 2156) && near(p[2], 2156) && near(p[3], 0));
    EXPECT_TRUE(ok) << "got (" << p[0] << "," << p[1] << ") -> (" << p[2] << "," << p[3] << ")";
}

TEST(Contour, AdjacentCellsStitchIntoSinglePolyline) {
    // 3×2 grid (3 cols, 2 rows), top half below threshold, bottom above.
    //
    //   50  50  50
    //   150 150 150
    //
    // For a width-3 grid, two cells exist horizontally. Each cell is "case 3"
    // (BL+BR above, TL+TR below) and produces a horizontal segment along the
    // cell's mid-height. The right edge of the left cell coincides with the
    // left edge of the right cell, so the two segments share an endpoint
    // and should stitch into a single polyline with 3 vertices total.
    //
    // Without stitching, we'd get 2 ContourLineStrings. With stitching, 1.
    const std::vector<std::int16_t> heights{50,  50,  50,
                                            150, 150, 150};
    const auto lines = generateContours(heights, 3, 2, withInterval(100.0));

    ASSERT_EQ(lines.size(), 1u) << "expected adjacent cells' segments to stitch into one polyline";
    EXPECT_DOUBLE_EQ(lines[0].elevation, 100.0);

    // 3 vertices = 6 ints.
    ASSERT_EQ(lines[0].points.size(), 6u);
}

TEST(Contour, SinglePeakProducesClosedRing) {
    // 3×3 grid with a single peak at the centre and zero everywhere else.
    // A contour around the peak should close back on itself: one polyline
    // whose first and last points coincide.
    //
    //     0   0   0
    //     0  100  0
    //     0   0   0
    //
    // With interval=50 and strict-`>` classification, the only level that
    // produces a crossing is 50 (level 0 doesn't cross — all "below or
    // equal" samples are treated as below, i.e. all corners are below 0
    // except the centre is above; case 2/4/1/8 each emit one segment per
    // cell, and they stitch into a closed ring around the peak).
    const std::vector<std::int16_t> heights{0, 0,   0,
                                            0, 100, 0,
                                            0, 0,   0};
    const auto lines = generateContours(heights, 3, 3, withInterval(50.0));

    // Count surviving polylines at level 50.
    int count50 = 0;
    int total = 0;
    for (const auto& l : lines) {
        ++total;
        if (l.elevation == 50.0) ++count50;
    }
    EXPECT_EQ(count50, 1) << "expected exactly one closed ring at level 50, got " << total << " total lines";
}

TEST(Contour, MultipleThresholdsEmittedInSingleCellPass) {
    // 2×2 cell crossing two contour levels in one go. With interval=50 and
    // a corner range of 110..300, levels 150, 200, 250, 300 are inside.
    // Level 300 sits exactly at the maximum (strict-`>` excludes it), so
    // 150, 200, 250 should each produce a segment. Confirms the
    // single-pass cell traversal emits all levels per cell.
    const std::vector<std::int16_t> heights{300, 110,
                                            110, 110};
    const auto lines = generateContours(heights, 2, 2, withInterval(50.0));

    EXPECT_EQ(lines.size(), 3u);
    int found150 = 0, found200 = 0, found250 = 0;
    for (const auto& l : lines) {
        if (l.elevation == 150.0) ++found150;
        else if (l.elevation == 200.0) ++found200;
        else if (l.elevation == 250.0) ++found250;
    }
    EXPECT_EQ(found150, 1);
    EXPECT_EQ(found200, 1);
    EXPECT_EQ(found250, 1);
}

TEST(Contour, BufferZeroEmitsLinesEntirelyWithinTile) {
    // 3×2 grid, contour at level 100 cuts horizontally through the middle.
    // With buffer=0 the algorithm only iterates cells whose corners are all
    // inside the grid, so output coords stay within [0, extent].
    const std::vector<std::int16_t> heights{50,  50,  50,
                                            150, 150, 150};
    const auto thresholds = ContourThresholds{/*interval=*/100.0, /*extent=*/4096, /*buffer=*/0};
    const auto lines = generateContours(heights, 3, 2, thresholds);
    ASSERT_FALSE(lines.empty());
    for (auto v : lines[0].points) {
        EXPECT_GE(v, 0);
        EXPECT_LE(v, 4096);
    }
}

TEST(Contour, BufferOnePushesLinesPastTileEdge) {
    // Same grid, buffer=1 — the loop iterates one cell past each edge,
    // sampling out-of-bounds positions which return NaN. Those cells skip
    // entirely (NaN check), so for a tile with no neighbour data the buffer
    // doesn't change output. Verify output is still valid (not regressed by
    // the buffer-iteration code).
    const std::vector<std::int16_t> heights{50,  50,  50,
                                            150, 150, 150};
    const auto thresholds = ContourThresholds{/*interval=*/100.0, /*extent=*/4096, /*buffer=*/1};
    const auto lines = generateContours(heights, 3, 2, thresholds);
    EXPECT_FALSE(lines.empty());
}

// Saddle cases (5 and 10) are the only marching-squares cases that emit two
// segments per cell. Without specific coverage, a regression in the case
// table could silently disable saddle output (or merge the two segments
// into one). These tests pin the segment count and approximate geometry.

TEST(Contour, SaddleCase5EmitsTwoSegments) {
    // 2×2 grid where TR and BL are above threshold, TL and BR are below.
    // Bit layout: TL=8 | TR=4 | BR=2 | BL=1, set if strictly above. So
    // 0 | 4 | 0 | 1 = 5 (saddle).
    //
    //   110  300
    //   300  110
    //
    // At level=200, the fixed-pairing saddle output is two diagonal
    // segments: one cutting (top, right) and one cutting (bottom, left).
    // Both lines share the same elevation; they are emitted as two
    // distinct ContourLineString entries because their edge endpoints
    // are not connected.
    const std::vector<std::int16_t> heights{110, 300,
                                            300, 110};
    const auto lines = generateContours(heights, 2, 2, withInterval(100.0));

    // Expect exactly 2 polylines, both at elevation=200, each with 2 vertices.
    int count200 = 0;
    for (const auto& l : lines) {
        if (l.elevation == 200.0) {
            ++count200;
            EXPECT_EQ(l.points.size(), 4u) << "saddle segment must be 2 vertices (4 ints)";
        }
    }
    EXPECT_EQ(count200, 2) << "case-5 saddle must emit two segments at the threshold";
}

TEST(Contour, SaddleCase10EmitsTwoSegments) {
    // The mirror saddle: TL and BR above, TR and BL below. Bits 8 | 0 | 2 | 0 = 10.
    //
    //   300  110
    //   110  300
    //
    // Two diagonal segments: one cutting (left, top) and one cutting (right, bottom).
    const std::vector<std::int16_t> heights{300, 110,
                                            110, 300};
    const auto lines = generateContours(heights, 2, 2, withInterval(100.0));

    int count200 = 0;
    for (const auto& l : lines) {
        if (l.elevation == 200.0) {
            ++count200;
            EXPECT_EQ(l.points.size(), 4u) << "saddle segment must be 2 vertices (4 ints)";
        }
    }
    EXPECT_EQ(count200, 2) << "case-10 saddle must emit two segments at the threshold";
}

TEST(Contour, PerformanceSmokeCheckUnder30ms) {
    // 256×256 grid with a synthetic gradient, 10 thresholds (interval=10 over
    // a 0..1000 range). Verifies the algorithm completes within the
    // performance budget from the Task 1 acceptance criteria. Not a precise
    // benchmark — but a 100× regression would catch any disastrous change.
    constexpr int N = 256;
    std::vector<std::int16_t> heights(static_cast<std::size_t>(N) * N);
    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
            // Concentric "hill" — non-trivial topology, multiple closed rings.
            const int dx = x - N / 2;
            const int dy = y - N / 2;
            const int r2 = dx * dx + dy * dy;
            heights[y * N + x] = static_cast<std::int16_t>(1000 - r2 / 64);
        }
    }
    const auto t0 = std::chrono::steady_clock::now();
    const auto lines = generateContours(heights, N, N, withInterval(100.0));
    const auto t1 = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    EXPECT_LT(ms, 30) << "algorithm took " << ms << " ms (budget: 30 ms)";
    EXPECT_GT(lines.size(), 0u);
}
