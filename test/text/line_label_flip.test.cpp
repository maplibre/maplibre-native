#include <mln/test/util.hpp>

#include <mln/layout/symbol_projection.hpp>
#include <mln/math/angles.hpp>

#include <cmath>
#include <optional>

using namespace mln;

namespace {

// Whether a label whose unflipped reading direction points `degrees` counterclockwise from the
// screen's x axis (gl coordinates, y up: 90 = straight up) has to be drawn flipped.
bool needsFlip(float degrees, std::optional<bool> wasFlipped, float aspectRatio = 1.0f) {
    const float radians = util::deg2radf(degrees);
    const Point<float> first{0.0f, 0.0f};
    const Point<float> last{100.0f * std::cos(radians) / aspectRatio, 100.0f * std::sin(radians)};
    return lineLabelNeedsFlip(first, last, aspectRatio, wasFlipped);
}

} // namespace

TEST(LineLabelFlip, FirstDrawingFlipsWhatRunsRightToLeft) {
    // As without the margin, also right next to the vertical.
    EXPECT_FALSE(needsFlip(0.0f, std::nullopt));
    EXPECT_FALSE(needsFlip(45.0f, std::nullopt));
    EXPECT_FALSE(needsFlip(89.0f, std::nullopt));
    EXPECT_TRUE(needsFlip(91.0f, std::nullopt));
    EXPECT_TRUE(needsFlip(180.0f, std::nullopt));
    EXPECT_TRUE(needsFlip(225.0f, std::nullopt));
    EXPECT_TRUE(needsFlip(269.0f, std::nullopt));
    EXPECT_FALSE(needsFlip(271.0f, std::nullopt));
}

TEST(LineLabelFlip, KeepsItsDirectionWhileTheLineStaysCloseToTheVertical) {
    // Drawn unflipped, the map rotating a few degrees either way around the vertical of the line.
    for (float degrees : {85.0f, 92.0f, 95.0f, 105.0f, 265.0f, 275.0f, 255.0f}) {
        EXPECT_FALSE(needsFlip(degrees, false)) << degrees;
    }
    // Drawn flipped: it stays flipped through the same rotations.
    for (float degrees : {95.0f, 88.0f, 85.0f, 75.0f, 265.0f, 275.0f, 285.0f}) {
        EXPECT_TRUE(needsFlip(degrees, true)) << degrees;
    }
}

TEST(LineLabelFlip, TurnsOverOnceClearlyUpsideDown) {
    EXPECT_TRUE(needsFlip(90.0f + lineLabelFlipMarginDegrees + 1.0f, false));
    EXPECT_TRUE(needsFlip(270.0f - lineLabelFlipMarginDegrees - 1.0f, false));
    EXPECT_FALSE(needsFlip(90.0f - lineLabelFlipMarginDegrees - 1.0f, true));
    EXPECT_FALSE(needsFlip(270.0f + lineLabelFlipMarginDegrees + 1.0f, true));
}

TEST(LineLabelFlip, MeasuresTheMarginOnTheScreenNotInGlUnits) {
    // On a tall screen one gl unit across is much shorter than one up: 15 degrees from the vertical
    // on the screen is inside the margin, even though in raw gl units it would look like 30.
    EXPECT_FALSE(needsFlip(105.0f, false, 0.46f));
    EXPECT_TRUE(needsFlip(105.0f, true, 0.46f));
}
